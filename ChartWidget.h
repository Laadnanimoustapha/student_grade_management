#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include "GradeManager.h"

QT_CHARTS_USE_NAMESPACE

class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);
    void updateChart(const GradeManager::ClassStatistics& stats);

private:
    QChart* chart;
    QChartView* chartView;
    
    void setupChart();
};

#endif // CHARTWIDGET_H