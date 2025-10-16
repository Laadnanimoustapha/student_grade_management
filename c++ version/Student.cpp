#include "Student.h"
#include <algorithm>
#include <numeric>
#include <cmath>

Student::Student() : id(""), name("") {}

Student::Student(const QString& id, const QString& name, const QVector<float>& grades)
    : id(id), name(name), grades(grades) {}

float Student::getGPA() const {
    if (grades.empty()) {
        return 0.0f;
    }
    
    float sum = std::accumulate(grades.begin(), grades.end(), 0.0f);
    float average = sum / static_cast<float>(grades.size());
    float gpa = average / GPA_SCALE;
    
    // Clamp between 0.0 and 4.0
    if (gpa > 4.0f) gpa = 4.0f;
    if (gpa < 0.0f) gpa = 0.0f;
    
    return gpa;
}

void Student::addGrade(float grade) {
    if (grade >= 0.0f && grade <= 100.0f) {
        grades.append(grade);
    }
}

bool Student::removeGrade(int index) {
    if (index >= 0 && index < grades.size()) {
        grades.removeAt(index);
        return true;
    }
    return false;
}

bool Student::updateGrade(int index, float newGrade) {
    if (index >= 0 && index < grades.size() && newGrade >= 0.0f && newGrade <= 100.0f) {
        grades[index] = newGrade;
        return true;
    }
    return false;
}

float Student::getHighestGrade() const {
    if (grades.empty()) return 0.0f;
    return *std::max_element(grades.begin(), grades.end());
}

float Student::getLowestGrade() const {
    if (grades.empty()) return 0.0f;
    return *std::min_element(grades.begin(), grades.end());
}

float Student::getAverageGrade() const {
    if (grades.empty()) return 0.0f;
    float sum = std::accumulate(grades.begin(), grades.end(), 0.0f);
    return sum / grades.size();
}

QString Student::getGradeTrend() const {
    if (grades.size() < 2) {
        return "N/A";
    }
    
    int upCount = 0, downCount = 0, steadyCount = 0;
    for (int i = 1; i < grades.size(); ++i) {
        float diff = grades[i] - grades[i - 1];
        if (std::fabs(diff) < 1e-3f) {
            steadyCount++;
        } else if (diff > 0.0f) {
            upCount++;
        } else {
            downCount++;
        }
    }
    
    return QString("%1↑ %2↓ %3→").arg(upCount).arg(downCount).arg(steadyCount);
}

bool Student::isValid() const {
    return !id.isEmpty() && !name.isEmpty();
}