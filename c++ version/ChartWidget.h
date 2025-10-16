#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>

class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);
    void updateChart();

private:
    void setupChart();
};

#endif // CHARTWIDGET_H