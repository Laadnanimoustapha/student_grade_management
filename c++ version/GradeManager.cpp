#include "GradeManager.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <algorithm>
#include <numeric>

GradeManager::GradeManager(QObject* parent) : QObject(parent) {}

bool GradeManager::addStudent(const Student& student) {
    if (!student.isValid() || idExists(student.getId())) {
        return false;
    }
    
    students.append(student);
    emit dataChanged();
    emit studentAdded(student);
    return true;
}

bool GradeManager::updateStudent(const QString& oldId, const Student& updatedStudent) {
    if (!updatedStudent.isValid()) {
        return false;
    }
    
    Student* existing = findStudentInternal(oldId);
    if (!existing) {
        return false;
    }
    
    // Check if new ID conflicts with other students
    if (oldId != updatedStudent.getId() && idExists(updatedStudent.getId(), existing)) {
        return false;
    }
    
    *existing = updatedStudent;
    emit dataChanged();
    emit studentUpdated(updatedStudent);
    return true;
}

bool GradeManager::deleteStudent(const QString& id) {
    auto it = std::find_if(students.begin(), students.end(), 
                          [&](const Student& s) { return s.getId() == id; });
    
    if (it != students.end()) {
        students.erase(it);
        emit dataChanged();
        emit studentDeleted(id);
        return true;
    }
    return false;
}

Student* GradeManager::findStudent(const QString& id) {
    return findStudentInternal(id);
}

Student* GradeManager::findStudentInternal(const QString& id) {
    auto it = std::find_if(students.begin(), students.end(),
                          [&](const Student& s) { return s.getId() == id; });
    return it != students.end() ? &(*it) : nullptr;
}

QVector<Student> GradeManager::searchByName(const QString& name) const {
    QVector<Student> results;
    QString searchTerm = name.toLower();
    
    for (const auto& student : students) {
        if (student.getName().toLower().contains(searchTerm)) {
            results.append(student);
        }
    }
    return results;
}

QVector<Student> GradeManager::searchByGpaRange(float minGpa, float maxGpa) const {
    QVector<Student> results;
    
    for (const auto& student : students) {
        float gpa = student.getGPA();
        if (gpa >= minGpa && gpa <= maxGpa) {
            results.append(student);
        }
    }
    return results;
}

QVector<Student> GradeManager::getTopPerformers(int count) const {
    QVector<Student> sorted = students;
    std::sort(sorted.begin(), sorted.end(),
              [](const Student& a, const Student& b) {
                  return a.getGPA() > b.getGPA();
              });
    
    if (count < sorted.size()) {
        sorted.resize(count);
    }
    return sorted;
}

void GradeManager::sortByName(bool ascending) {
    std::sort(students.begin(), students.end(),
              [ascending](const Student& a, const Student& b) {
                  if (ascending) {
                      return a.getName().toLower() < b.getName().toLower();
                  } else {
                      return a.getName().toLower() > b.getName().toLower();
                  }
              });
    emit dataChanged();
}

void GradeManager::sortByGPA(bool descending) {
    std::sort(students.begin(), students.end(),
              [descending](const Student& a, const Student& b) {
                  if (descending) {
                      return a.getGPA() > b.getGPA();
                  } else {
                      return a.getGPA() < b.getGPA();
                  }
              });
    emit dataChanged();
}

void GradeManager::sortById(bool ascending) {
    std::sort(students.begin(), students.end(),
              [ascending](const Student& a, const Student& b) {
                  if (ascending) {
                      return a.getId() < b.getId();
                  } else {
                      return a.getId() > b.getId();
                  }
              });
    emit dataChanged();
}

GradeManager::ClassStatistics GradeManager::calculateStatistics() const {
    ClassStatistics stats;
    stats.totalStudents = students.size();
    
    if (students.empty()) {
        return stats;
    }
    
    double totalGpa = 0.0;
    double totalGrades = 0.0;
    int gradeCount = 0;
    const Student* topStudent = nullptr;
    const Student* bottomStudent = nullptr;
    
    // Initialize grade distribution buckets: 90-100, 80-89, 70-79, 60-69, 0-59
    stats.gradeDistribution = QVector<int>(5, 0);
    
    for (const auto& student : students) {
        float gpa = student.getGPA();
        totalGpa += gpa;
        
        // Update top/bottom students
        if (!topStudent || gpa > topStudent->getGPA()) {
            topStudent = &student;
        }
        if (!bottomStudent || gpa < bottomStudent->getGPA()) {
            bottomStudent = &student;
        }
        
        // Count honors and probation
        if (gpa >= 3.5f) {
            stats.honorsCount++;
        }
        if (gpa < 2.0f) {
            stats.probationCount++;
        }
        
        // Calculate grade distribution
        for (float grade : student.getGrades()) {
            totalGrades += grade;
            gradeCount++;
            
            if (grade >= 90.0f) stats.gradeDistribution[0]++;
            else if (grade >= 80.0f) stats.gradeDistribution[1]++;
            else if (grade >= 70.0f) stats.gradeDistribution[2]++;
            else if (grade >= 60.0f) stats.gradeDistribution[3]++;
            else stats.gradeDistribution[4]++;
        }
    }
    
    stats.averageGPA = totalGpa / students.size();
    stats.averageGrade = gradeCount > 0 ? totalGrades / gradeCount : 0.0;
    
    if (topStudent) {
        stats.topStudentName = topStudent->getName();
        stats.topStudentId = topStudent->getId();
        stats.topStudentGPA = topStudent->getGPA();
    }
    
    if (bottomStudent) {
        stats.bottomStudentName = bottomStudent->getName();
        stats.bottomStudentId = bottomStudent->getId();
        stats.bottomStudentGPA = bottomStudent->getGPA();
    }
    
    return stats;
}

bool GradeManager::saveToFile(const QString& filename) {
    QJsonArray studentsArray;
    
    for (const auto& student : students) {
        studentsArray.append(studentToJson(student));
    }
    
    QJsonObject root;
    root["version"] = "2.0";
    root["students"] = studentsArray;
    
    QJsonDocument doc(root);
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool GradeManager::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray studentsArray = root["students"].toArray();
    
    students.clear();
    for (const auto& studentValue : studentsArray) {
        Student student = jsonToStudent(studentValue.toObject());
        if (student.isValid()) {
            students.append(student);
        }
    }
    
    emit dataChanged();
    return true;
}

bool GradeManager::exportToCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << "ID,Name,GPA,Grades\n";
    
    for (const auto& student : students) {
        out << student.getId() << ","
            << "\"" << student.getName() << "\","
            << QString::number(student.getGPA(), 'f', 2) << ","
            << "\"";
        
        const auto& grades = student.getGrades();
        for (int i = 0; i < grades.size(); ++i) {
            out << QString::number(grades[i], 'f', 1);
            if (i < grades.size() - 1) {
                out << ";";
            }
        }
        out << "\"\n";
    }
    
    return true;
}

bool GradeManager::importFromCSV(const QString& filename) {
    // Implementation for CSV import
    // This is a simplified version - you'd want more robust parsing
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString header = in.readLine(); // Skip header
    
    QVector<Student> importedStudents;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        
        if (parts.size() >= 3) {
            QString id = parts[0].trimmed();
            QString name = parts[1].trimmed().remove('"');
            QVector<float> grades;
            
            // Parse grades if available
            if (parts.size() > 3) {
                QString gradesStr = parts[3].trimmed().remove('"');
                QStringList gradeList = gradesStr.split(';');
                for (const QString& gradeStr : gradeList) {
                    bool ok;
                    float grade = gradeStr.toFloat(&ok);
                    if (ok && grade >= 0 && grade <= 100) {
                        grades.append(grade);
                    }
                }
            }
            
            Student student(id, name, grades);
            if (student.isValid() && !idExists(id)) {
                importedStudents.append(student);
            }
        }
    }
    
    if (!importedStudents.isEmpty()) {
        students.append(importedStudents);
        emit dataChanged();
        return true;
    }
    
    return false;
}

void GradeManager::exportProgressReports(const QString& directory) {
    QDir dir;
    if (!dir.exists(directory)) {
        dir.mkpath(directory);
    }
    
    for (const auto& student : students) {
        QString filename = QString("%1/%2_report.txt").arg(directory, student.getId());
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Student Progress Report\n";
            out << "======================\n\n";
            out << "Name: " << student.getName() << "\n";
            out << "ID: " << student.getId() << "\n";
            out << "GPA: " << QString::number(student.getGPA(), 'f', 2) << "\n";
            out << "Grade Trend: " << student.getGradeTrend() << "\n";
            out << "Highest Grade: " << QString::number(student.getHighestGrade(), 'f', 1) << "\n";
            out << "Lowest Grade: " << QString::number(student.getLowestGrade(), 'f', 1) << "\n";
            out << "Average Grade: " << QString::number(student.getAverageGrade(), 'f', 1) << "\n\n";
            
            out << "Grades:\n";
            const auto& grades = student.getGrades();
            for (int i = 0; i < grades.size(); ++i) {
                out << "  Assignment " << (i + 1) << ": " 
                    << QString::number(grades[i], 'f', 1) << "\n";
            }
        }
    }
}

bool GradeManager::idExists(const QString& id, const Student* exclude) const {
    for (const auto& student : students) {
        if (&student != exclude && student.getId() == id) {
            return true;
        }
    }
    return false;
}

QJsonObject GradeManager::studentToJson(const Student& student) const {
    QJsonObject obj;
    obj["id"] = student.getId();
    obj["name"] = student.getName();
    
    QJsonArray gradesArray;
    for (float grade : student.getGrades()) {
        gradesArray.append(grade);
    }
    obj["grades"] = gradesArray;
    
    return obj;
}

Student GradeManager::jsonToStudent(const QJsonObject& json) const {
    QString id = json["id"].toString();
    QString name = json["name"].toString();
    
    QVector<float> grades;
    QJsonArray gradesArray = json["grades"].toArray();
    for (const auto& gradeValue : gradesArray) {
        grades.append(static_cast<float>(gradeValue.toDouble()));
    }
    
    return Student(id, name, grades);
}