/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#ifndef RUNTIMEWINDOW_H
#define RUNTIMEWINDOW_H

#include <QDialog>
#include "qcustomplot.h"
#include <string>

enum class config {forward, optimize, inverse, mcmc};

namespace Ui {
class RunTimeWindow;
}

class RunTimeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RunTimeWindow(QWidget *parent = nullptr, config cnfg=config::forward);
    ~RunTimeWindow();
    QCustomPlot *plot = nullptr;
    QCustomPlot *plot2 = nullptr;
    void AppendText(const QString &s);
    void AppendText(const std::string &s);
    void AppendtoDetails(const QString &s);
    void AddDataPoint(const double &t, const double value, int graph_no=0);
    void Replot();
    void SetProgress(const double &val);
    void SetProgress2(const double &val);
    void SetXRange(const double &tstart, const double &tend, int plotno=0);
    void SetYRange(const double &ymin, const double &ymax);
    void AppendErrorMessage(const QString &s);
    void SetUp(config);
    bool detailson = false;
    bool stoptriggered = false;
private:
    Ui::RunTimeWindow *ui;

public slots:
    void showdetails();
    void stop_triggered();
};

#endif // RUNTIMEWINDOW_H
