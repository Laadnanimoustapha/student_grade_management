#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QProgressBar>
#include "GradeManager.h"
#include "ChartWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddStudent();
    void onUpdateStudent();
    void onDeleteStudent();
    void onSearchStudents();
    void onSortStudents();
    void onSaveData();
    void onLoadData();
    void onExportCSV();
    void onImportCSV();
    void onExportReports();
    void onShowTopPerformers();
    void onStudentSelected(int row, int column);
    void onDataChanged();
    void updateStatistics();
    void onFilterChanged();

private:
    void setupUI();
    void setupConnections();
    void refreshStudentTable();
    void refreshStatistics();
    void showStudentDetails(const Student& student);
    void clearStudentForm();
    void loadStyles();
    
    // UI Components
    QTabWidget* mainTabWidget;
    
    // Student Management Tab
    QTableWidget* studentTable;
    QLineEdit* idEdit;
    QLineEdit* nameEdit;
    QListWidget* gradesList;
    QLineEdit* newGradeEdit;
    QPushButton* addGradeBtn;
    QPushButton* removeGradeBtn;
    QPushButton* addStudentBtn;
    QPushButton* updateStudentBtn;
    QPushButton* deleteStudentBtn;
    
    // Search Tab
    QLineEdit* searchEdit;
    QComboBox* searchTypeCombo;
    QTableWidget* searchResultsTable;
    QDoubleSpinBox* minGpaSpin;
    QDoubleSpinBox* maxGpaSpin;
    
    // Statistics Tab
    ChartWidget* chartWidget;
    QLabel* totalStudentsLabel;
    QLabel* averageGpaLabel;
    QLabel* averageGradeLabel;
    QLabel* honorsCountLabel;
    QLabel* probationCountLabel;
    QLabel* topStudentLabel;
    QLabel* bottomStudentLabel;
    QProgressBar* overallProgress;
    
    // Data
    GradeManager* gradeManager;
    QString currentStudentId;
};

#endif // MAINWINDOW_H