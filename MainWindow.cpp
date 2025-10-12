#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QStyledItemDelegate>

// Custom delegate for coloring GPA cells
class GpaDelegate : public QStyledItemDelegate {
public:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        if (index.column() == 2) { // GPA column
            float gpa = index.data(Qt::DisplayRole).toFloat();
            if (gpa >= 3.5) {
                option->backgroundBrush = QBrush(QColor("#E8F5E8"));
            } else if (gpa < 2.0) {
                option->backgroundBrush = QBrush(QColor("#FFEBEE"));
            }
        }
    }
};

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), gradeManager(new GradeManager(this)) {
    setupUI();
    setupConnections();
    loadStyles();
    refreshStudentTable();
    updateStatistics();
    
    setWindowTitle("Student Grade Management System");
    setMinimumSize(1200, 800);
    resize(1400, 900);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);
    
    // Student Management Tab
    QWidget* managementTab = new QWidget();
    QVBoxLayout* managementLayout = new QVBoxLayout(managementTab);
    
    // Student Table
    studentTable = new QTableWidget();
    studentTable->setColumnCount(4);
    studentTable->setHorizontalHeaderLabels({"ID", "Name", "GPA", "Grades Count"});
    studentTable->horizontalHeader()->setStretchLastSection(true);
    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->setItemDelegate(new GpaDelegate());
    
    // Student Form
    QGroupBox* formGroup = new QGroupBox("Student Information");
    QFormLayout* formLayout = new QFormLayout(formGroup);
    
    idEdit = new QLineEdit();
    nameEdit = new QLineEdit();
    
    formLayout->addRow("Student ID:", idEdit);
    formLayout->addRow("Name:", nameEdit);
    
    // Grades Management
    QGroupBox* gradesGroup = new QGroupBox("Grades Management");
    QVBoxLayout* gradesLayout = new QVBoxLayout(gradesGroup);
    
    gradesList = new QListWidget();
    QHBoxLayout* gradeInputLayout = new QHBoxLayout();
    newGradeEdit = new QLineEdit();
    newGradeEdit->setPlaceholderText("Enter grade (0-100)");
    addGradeBtn = new QPushButton("Add Grade");
    removeGradeBtn = new QPushButton("Remove Selected");
    
    gradeInputLayout->addWidget(newGradeEdit);
    gradeInputLayout->addWidget(addGradeBtn);
    gradeInputLayout->addWidget(removeGradeBtn);
    
    gradesLayout->addWidget(gradesList);
    gradesLayout->addLayout(gradeInputLayout);
    
    // Action Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addStudentBtn = new QPushButton("Add Student");
    updateStudentBtn = new QPushButton("Update Student");
    deleteStudentBtn = new QPushButton("Delete Student");
    updateStudentBtn->setEnabled(false);
    deleteStudentBtn->setEnabled(false);
    
    buttonLayout->addWidget(addStudentBtn);
    buttonLayout->addWidget(updateStudentBtn);
    buttonLayout->addWidget(deleteStudentBtn);
    buttonLayout->addStretch();
    
    // Add widgets to management layout
    managementLayout->addWidget(studentTable);
    managementLayout->addWidget(formGroup);
    managementLayout->addWidget(gradesGroup);
    managementLayout->addLayout(buttonLayout);
    
    // Search Tab
    QWidget* searchTab = new QWidget();
    QVBoxLayout* searchLayout = new QVBoxLayout(searchTab);
    
    QHBoxLayout* searchControlLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Enter search term...");
    searchTypeCombo = new QComboBox();
    searchTypeCombo->addItems({"Search by Name", "Search by ID", "Search by GPA Range"});
    
    minGpaSpin = new QDoubleSpinBox();
    minGpaSpin->setRange(0.0, 4.0);
    minGpaSpin->setValue(0.0);
    minGpaSpin->setSingleStep(0.1);
    minGpaSpin->setPrefix("Min: ");
    
    maxGpaSpin = new QDoubleSpinBox();
    maxGpaSpin->setRange(0.0, 4.0);
    maxGpaSpin->setValue(4.0);
    maxGpaSpin->setSingleStep(0.1);
    maxGpaSpin->setPrefix("Max: ");
    
    QPushButton* searchBtn = new QPushButton("Search");
    
    searchControlLayout->addWidget(searchTypeCombo);
    searchControlLayout->addWidget(searchEdit);
    searchControlLayout->addWidget(minGpaSpin);
    searchControlLayout->addWidget(maxGpaSpin);
    searchControlLayout->addWidget(searchBtn);
    searchControlLayout->addStretch();
    
    searchResultsTable = new QTableWidget();
    searchResultsTable->setColumnCount(4);
    searchResultsTable->setHorizontalHeaderLabels({"ID", "Name", "GPA", "Grades Count"});
    searchResultsTable->horizontalHeader()->setStretchLastSection(true);
    
    searchLayout->addLayout(searchControlLayout);
    searchLayout->addWidget(searchResultsTable);
    
    // Statistics Tab
    QWidget* statsTab = new QWidget();
    QHBoxLayout* statsLayout = new QHBoxLayout(statsTab);
    
    // Left side: Chart
    chartWidget = new ChartWidget();
    
    // Right side: Statistics
    QWidget* statsPanel = new QWidget();
    statsPanel->setMaximumWidth(400);
    QVBoxLayout* statsPanelLayout = new QVBoxLayout(statsPanel);
    
    QGroupBox* numbersGroup = new QGroupBox("Class Statistics");
    QFormLayout* numbersLayout = new QFormLayout(numbersGroup);
    
    totalStudentsLabel = new QLabel("0");
    averageGpaLabel = new QLabel("0.00");
    averageGradeLabel = new QLabel("0.00");
    honorsCountLabel = new QLabel("0");
    probationCountLabel = new QLabel("0");
    topStudentLabel = new QLabel("N/A");
    bottomStudentLabel = new QLabel("N/A");
    
    numbersLayout->addRow("Total Students:", totalStudentsLabel);
    numbersLayout->addRow("Average GPA:", averageGpaLabel);
    numbersLayout->addRow("Average Grade:", averageGradeLabel);
    numbersLayout->addRow("Students with Honors:", honorsCountLabel);
    numbersLayout->addRow("On Academic Probation:", probationCountLabel);
    numbersLayout->addRow("Top Student:", topStudentLabel);
    numbersLayout->addRow("Lowest GPA Student:", bottomStudentLabel);
    
    overallProgress = new QProgressBar();
    overallProgress->setRange(0, 100);
    overallProgress->setValue(0);
    overallProgress->setFormat("Overall Progress: %p%");
    
    QPushButton* topPerformersBtn = new QPushButton("Show Top 10 Performers");
    QPushButton* exportReportsBtn = new QPushButton("Export Progress Reports");
    
    statsPanelLayout->addWidget(numbersGroup);
    statsPanelLayout->addWidget(overallProgress);
    statsPanelLayout->addWidget(topPerformersBtn);
    statsPanelLayout->addWidget(exportReportsBtn);
    statsPanelLayout->addStretch();
    
    statsLayout->addWidget(chartWidget);
    statsLayout->addWidget(statsPanel);
    
    // Add tabs
    mainTabWidget->addTab(managementTab, "Student Management");
    mainTabWidget->addTab(searchTab, "Search & Filter");
    mainTabWidget->addTab(statsTab, "Statistics & Reports");
    
    // Menu Bar
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    QMenu* fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("Load Data", this, &MainWindow::onLoadData);
    fileMenu->addAction("Save Data", this, &MainWindow::onSaveData);
    fileMenu->addSeparator();
    fileMenu->addAction("Import CSV", this, &MainWindow::onImportCSV);
    fileMenu->addAction("Export CSV", this, &MainWindow::onExportCSV);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QMainWindow::close);
    
    QMenu* toolsMenu = menuBar->addMenu("Tools");
    toolsMenu->addAction("Sort by Name", [this]() { gradeManager->sortByName(true); });
    toolsMenu->addAction("Sort by GPA", [this]() { gradeManager->sortByGPA(true); });
    toolsMenu->addAction("Show Top Performers", this, &MainWindow::onShowTopPerformers);
}

void MainWindow::setupConnections() {
    connect(addStudentBtn, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    connect(updateStudentBtn, &QPushButton::clicked, this, &MainWindow::onUpdateStudent);
    connect(deleteStudentBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStudent);
    connect(addGradeBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        float grade = newGradeEdit->text().toFloat(&ok);
        if (ok && grade >= 0 && grade <= 100) {
            gradesList->addItem(QString::number(grade, 'f', 1));
            newGradeEdit->clear();
        } else {
            QMessageBox::warning(this, "Invalid Grade", "Please enter a valid grade between 0 and 100.");
        }
    });
    
    connect(removeGradeBtn, &QPushButton::clicked, this, [this]() {
        delete gradesList->takeItem(gradesList->currentRow());
    });
    
    connect(studentTable, &QTableWidget::cellClicked, this, &MainWindow::onStudentSelected);
    connect(gradeManager, &GradeManager::dataChanged, this, &MainWindow::onDataChanged);
    
    // Search connections
    QPushButton* searchBtn = searchTab()->findChild<QPushButton*>();
    if (searchBtn) {
        connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchStudents);
    }
    
    connect(searchTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFilterChanged);
}

void MainWindow::loadStyles() {
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        this->setStyleSheet(styleSheet);
    }
}

void MainWindow::onAddStudent() {
    QString id = idEdit->text().trimmed();
    QString name = nameEdit->text().trimmed();
    
    if (id.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter both ID and name.");
        return;
    }
    
    if (gradeManager->idExists(id)) {
        QMessageBox::warning(this, "Duplicate ID", "A student with this ID already exists.");
        return;
    }
    
    QVector<float> grades;
    for (int i = 0; i < gradesList->count(); ++i) {
        grades.append(gradesList->item(i)->text().toFloat());
    }
    
    Student student(id, name, grades);
    if (gradeManager->addStudent(student)) {
        clearStudentForm();
        QMessageBox::information(this, "Success", "Student added successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to add student.");
    }
}

void MainWindow::onUpdateStudent() {
    if (currentStudentId.isEmpty()) return;
    
    QString newId = idEdit->text().trimmed();
    QString newName = nameEdit->text().trimmed();
    
    if (newId.isEmpty() || newName.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter both ID and name.");
        return;
    }
    
    QVector<float> grades;
    for (int i = 0; i < gradesList->count(); ++i) {
        grades.append(gradesList->item(i)->text().toFloat());
    }
    
    Student updatedStudent(newId, newName, grades);
    if (gradeManager->updateStudent(currentStudentId, updatedStudent)) {
        currentStudentId = newId;
        QMessageBox::information(this, "Success", "Student updated successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to update student.");
    }
}

void MainWindow::onDeleteStudent() {
    if (currentStudentId.isEmpty()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        "Are you sure you want to delete this student?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (gradeManager->deleteStudent(currentStudentId)) {
            clearStudentForm();
            QMessageBox::information(this, "Success", "Student deleted successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete student.");
        }
    }
}

void MainWindow::onSearchStudents() {
    int searchType = searchTypeCombo->currentIndex();
    QVector<Student> results;
    
    switch (searchType) {
    case 0: // Search by Name
        results = gradeManager->searchByName(searchEdit->text());
        break;
    case 1: // Search by ID
        if (Student* student = gradeManager->findStudent(searchEdit->text())) {
            results.append(*student);
        }
        break;
    case 2: // Search by GPA Range
        results = gradeManager->searchByGpaRange(minGpaSpin->value(), maxGpaSpin->value());
        break;
    }
    
    // Display results
    searchResultsTable->setRowCount(results.size());
    for (int i = 0; i < results.size(); ++i) {
        const Student& student = results[i];
        searchResultsTable->setItem(i, 0, new QTableWidgetItem(student.getId()));
        searchResultsTable->setItem(i, 1, new QTableWidgetItem(student.getName()));
        searchResultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(student.getGPA(), 'f', 2)));
        searchResultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(student.getGrades().size())));
    }
}

void MainWindow::onSortStudents() {
    // Sorting is handled through menu actions
}

void MainWindow::onSaveData() {
    QString filename = QFileDialog::getSaveFileName(this, "Save Data", "student_data.json", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        if (gradeManager->saveToFile(filename)) {
            QMessageBox::information(this, "Success", "Data saved successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to save data.");
        }
    }
}

void MainWindow::onLoadData() {
    QString filename = QFileDialog::getOpenFileName(this, "Load Data", "", "JSON Files (*.json)");
    if (!filename.isEmpty()) {
        if (gradeManager->loadFromFile(filename)) {
            refreshStudentTable();
            updateStatistics();
            QMessageBox::information(this, "Success", "Data loaded successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to load data.");
        }
    }
}

void MainWindow::onExportCSV() {
    QString filename = QFileDialog::getSaveFileName(this, "Export CSV", "students.csv", "CSV Files (*.csv)");
    if (!filename.isEmpty()) {
        if (gradeManager->exportToCSV(filename)) {
            QMessageBox::information(this, "Success", "Data exported to CSV successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to export data.");
        }
    }
}

void MainWindow::onImportCSV() {
    QString filename = QFileDialog::getOpenFileName(this, "Import CSV", "", "CSV Files (*.csv)");
    if (!filename.isEmpty()) {
        if (gradeManager->importFromCSV(filename)) {
            refreshStudentTable();
            updateStatistics();
            QMessageBox::information(this, "Success", "Data imported from CSV successfully!");
        } else {
            QMessageBox::warning(this, "Error", "Failed to import data.");
        }
    }
}

void MainWindow::onExportReports() {
    QString directory = QFileDialog::getExistingDirectory(this, "Select Reports Directory");
    if (!directory.isEmpty()) {
        gradeManager->exportProgressReports(directory);
        QMessageBox::information(this, "Success", "Progress reports exported successfully!");
    }
}

void MainWindow::onShowTopPerformers() {
    auto topPerformers = gradeManager->getTopPerformers(10);
    
    QString message = "Top 10 Performers:\n\n";
    for (int i = 0; i < topPerformers.size(); ++i) {
        const auto& student = topPerformers[i];
        message += QString("%1. %2 (ID: %3) - GPA: %4\n")
                      .arg(i + 1)
                      .arg(student.getName())
                      .arg(student.getId())
                      .arg(student.getGPA(), 0, 'f', 2);
    }
    
    QMessageBox::information(this, "Top Performers", message);
}

void MainWindow::onStudentSelected(int row, int column) {
    Q_UNUSED(column)
    
    if (row < 0 || row >= studentTable->rowCount()) return;
    
    QString studentId = studentTable->item(row, 0)->text();
    Student* student = gradeManager->findStudent(studentId);
    
    if (student) {
        showStudentDetails(*student);
        currentStudentId = studentId;
        updateStudentBtn->setEnabled(true);
        deleteStudentBtn->setEnabled(true);
    }
}

void MainWindow::onDataChanged() {
    refreshStudentTable();
    updateStatistics();
}

void MainWindow::updateStatistics() {
    refreshStatistics();
}

void MainWindow::onFilterChanged() {
    int searchType = searchTypeCombo->currentIndex();
    searchEdit->setVisible(searchType != 2);
    minGpaSpin->setVisible(searchType == 2);
    maxGpaSpin->setVisible(searchType == 2);
}

void MainWindow::refreshStudentTable() {
    studentTable->setRowCount(0);
    
    const auto& allStudents = gradeManager->getAllStudents();
    studentTable->setRowCount(allStudents.size());
    
    for (int i = 0; i < allStudents.size(); ++i) {
        const Student& student = allStudents[i];
        studentTable->setItem(i, 0, new QTableWidgetItem(student.getId()));
        studentTable->setItem(i, 1, new QTableWidgetItem(student.getName()));
        studentTable->setItem(i, 2, new QTableWidgetItem(QString::number(student.getGPA(), 'f', 2)));
        studentTable->setItem(i, 3, new QTableWidgetItem(QString::number(student.getGrades().size())));
    }
}

void MainWindow::refreshStatistics() {
    auto stats = gradeManager->calculateStatistics();
    
    totalStudentsLabel->setText(QString::number(stats.totalStudents));
    averageGpaLabel->setText(QString::number(stats.averageGPA, 'f', 2));
    averageGradeLabel->setText(QString::number(stats.averageGrade, 'f', 1));
    honorsCountLabel->setText(QString::number(stats.honorsCount));
    probationCountLabel->setText(QString::number(stats.probationCount));
    
    if (!stats.topStudentName.isEmpty()) {
        topStudentLabel->setText(QString("%1 (%2) - GPA: %3")
                                    .arg(stats.topStudentName)
                                    .arg(stats.topStudentId)
                                    .arg(stats.topStudentGPA, 0, 'f', 2));
    } else {
        topStudentLabel->setText("N/A");
    }
    
    if (!stats.bottomStudentName.isEmpty()) {
        bottomStudentLabel->setText(QString("%1 (%2) - GPA: %3")
                                       .arg(stats.bottomStudentName)
                                       .arg(stats.bottomStudentId)
                                       .arg(stats.bottomStudentGPA, 0, 'f', 2));
    } else {
        bottomStudentLabel->setText("N/A");
    }
    
    // Update progress bar based on average GPA (0-4 scale to 0-100)
    overallProgress->setValue(static_cast<int>((stats.averageGPA / 4.0) * 100));
    
    // Update chart
    chartWidget->updateChart(stats);
}

void MainWindow::showStudentDetails(const Student& student) {
    idEdit->setText(student.getId());
    nameEdit->setText(student.getName());
    
    gradesList->clear();
    for (float grade : student.getGrades()) {
        gradesList->addItem(QString::number(grade, 'f', 1));
    }
}

void MainWindow::clearStudentForm() {
    idEdit->clear();
    nameEdit->clear();
    gradesList->clear();
    newGradeEdit->clear();
    currentStudentId.clear();
    updateStudentBtn->setEnabled(false);
    deleteStudentBtn->setEnabled(false);
}

// Helper function to get search tab
QWidget* MainWindow::searchTab() {
    return mainTabWidget->widget(1);
}