#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QVector>
#include <QMetaType>

class Student {
public:
    Student();
    Student(const QString& id, const QString& name, const QVector<float>& grades = QVector<float>());
    
    // Getters
    QString getId() const { return id; }
    QString getName() const { return name; }
    QVector<float> getGrades() const { return grades; }
    float getGPA() const;
    
    // Setters
    void setId(const QString& newId) { id = newId; }
    void setName(const QString& newName) { name = newName; }
    void setGrades(const QVector<float>& newGrades) { grades = newGrades; }
    void addGrade(float grade);
    bool removeGrade(int index);
    bool updateGrade(int index, float newGrade);
    
    // Statistics
    float getHighestGrade() const;
    float getLowestGrade() const;
    float getAverageGrade() const;
    QString getGradeTrend() const;
    
    // Validation
    bool isValid() const;
    
private:
    QString id;
    QString name;
    QVector<float> grades;
    
    static constexpr float GPA_SCALE = 25.0f;
};

Q_DECLARE_METATYPE(Student)

#endif // STUDENT_H