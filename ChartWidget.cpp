#include "ChartWidget.h"
#include <QVBoxLayout>

ChartWidget::ChartWidget(QWidget* parent) : QWidget(parent) {
    setupChart();
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    setLayout(layout);
}

void ChartWidget::setupChart() {
    chart = new QChart();
    chart->setTitle("Class Statistics");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}

void ChartWidget::updateChart(const GradeManager::ClassStatistics& stats) {
    chart->removeAllSeries();
    
    // Create bar series for grade distribution
    QBarSeries* series = new QBarSeries();
    
    QStringList categories = {"90-100", "80-89", "70-79", "60-69", "0-59"};
    QBarSet* set = new QBarSet("Grade Distribution");
    
    for (int count : stats.gradeDistribution) {
        *set << count;
    }
    
    series->append(set);
    chart->addSeries(series);
    
    // Create category axis
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    // Create value axis
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Number of Grades");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    // Add GPA pie chart
    if (stats.totalStudents > 0) {
        QPieSeries* pieSeries = new QPieSeries();
        pieSeries->setHoleSize(0.4);
        
        int honors = stats.honorsCount;
        int probation = stats.probationCount;
        int regular = stats.totalStudents - honors - probation;
        
        if (honors > 0) {
            QPieSlice* slice = pieSeries->append("Honors (≥3.5)", honors);
            slice->setColor(QColor("#4CAF50"));
        }
        if (regular > 0) {
            QPieSlice* slice = pieSeries->append("Regular (2.0-3.49)", regular);
            slice->setColor(QColor("#2196F3"));
        }
        if (probation > 0) {
            QPieSlice* slice = pieSeries->append("Probation (<2.0)", probation);
            slice->setColor(QColor("#F44336"));
        }
        
        chart->addSeries(pieSeries);
    }
}