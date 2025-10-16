#include "ChartWidget.h"
#include <QVBoxLayout>
#include <QLabel>

ChartWidget::ChartWidget(QWidget* parent) : QWidget(parent) {
    setupChart();
}

void ChartWidget::setupChart() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("Charts require QtCharts module. Install qt6-charts or qt5-charts package.");
    layout->addWidget(label);
    setLayout(layout);
}

void ChartWidget::updateChart() {
    // Simplified - no chart functionality without QtCharts
}