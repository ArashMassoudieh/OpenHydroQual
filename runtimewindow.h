#ifndef RUNTIMEWINDOW_H
#define RUNTIMEWINDOW_H

#include <QDialog>
#include "qcustomplot.h"

namespace Ui {
class RunTimeWindow;
}

class RunTimeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RunTimeWindow(QWidget *parent = nullptr);
    ~RunTimeWindow();
    QCustomPlot *plot;
    void AppendText(const QString &s);
    void AppendtoDetails(const QString &s);
    void AddDataPoint(const double &t, const double value);
    void SetProgress(const double &val);
    void SetProgress2(const double &val);
    void SetXRange(const double &tstart, const double &tend);
    void SetYRange(const double &ymin, const double &ymax);
    void AppendErrorMessage(const QString &s);
    void SetUpForForwardRun();
	void SetUpForInverseRun();
    bool detailson = false;
    bool stoptriggered = false;
private:
    Ui::RunTimeWindow *ui;

public slots:
    void showdetails();
    void stop_triggered();
};

#endif // RUNTIMEWINDOW_H
