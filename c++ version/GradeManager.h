#ifndef GRADEMANAGER_H
#define GRADEMANAGER_H

#include "Student.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

class GradeManager : public QObject {
    Q_OBJECT

public:
    explicit GradeManager(QObject* parent = nullptr);
    
    // Student management
    bool addStudent(const Student& student);
    bool updateStudent(const QString& oldId, const Student& updatedStudent);
    bool deleteStudent(const QString& id);
    Student* findStudent(const QString& id);
    QVector<Student> getAllStudents() const { return students; }
    
    // Search and filter
    QVector<Student> searchByName(const QString& name) const;
    QVector<Student> searchByGpaRange(float minGpa, float maxGpa) const;
    QVector<Student> getTopPerformers(int count) const;
    
    // Sorting
    void sortByName(bool ascending = true);
    void sortByGPA(bool descending = true);
    void sortById(bool ascending = true);
    
    // Statistics
    struct ClassStatistics {
        int totalStudents = 0;
        double averageGPA = 0.0;
        double averageGrade = 0.0;
        int honorsCount = 0;      // GPA >= 3.5
        int probationCount = 0;   // GPA < 2.0
        QString topStudentName;
        QString topStudentId;
        float topStudentGPA = 0.0;
        QString bottomStudentName;
        QString bottomStudentId;
        float bottomStudentGPA = 0.0;
        QVector<int> gradeDistribution; // 90-100, 80-89, 70-79, 60-69, 0-59
    };
    
    ClassStatistics calculateStatistics() const;
    
    // File operations
    bool saveToFile(const QString& filename = "student_data.json");
    bool loadFromFile(const QString& filename = "student_data.json");
    bool exportToCSV(const QString& filename = "students.csv");
    bool importFromCSV(const QString& filename = "students.csv");
    void exportProgressReports(const QString& directory = "reports");
    
    // Validation
    bool idExists(const QString& id, const Student* exclude = nullptr) const;
    
signals:
    void dataChanged();
    void studentAdded(const Student& student);
    void studentUpdated(const Student& student);
    void studentDeleted(const QString& id);

private:
    QVector<Student> students;
    
    Student* findStudentInternal(const QString& id);
    QJsonObject studentToJson(const Student& student) const;
    Student jsonToStudent(const QJsonObject& json) const;
};

#endif // GRADEMANAGER_H