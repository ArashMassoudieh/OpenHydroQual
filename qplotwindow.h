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


#ifndef QPLOTWINDOW_H
#define QPLOTWINDOW_H

#include <QDialog>
#include <TimeSeries.h>
#include <TimeSeriesSet.h>
#include <qplotter.h>

#define outputtimeseriesprecision double

class MainWindow;
class ChartView;
/*
struct plotformat{
    Qt::GlobalColor color = Qt::red;
    QBrush brush = QBrush(QColor(240, 255, 200));
    //QCPGraph *fillGraph = 0;
    //QCPGraph::LineStyle lineStyle = QCPGraph::lsLine;
    //QCPScatterStyle::ScatterShape scatterStyle = QCPScatterStyle::ssNone;// ssDisc;
    bool legend = true;
    Qt::PenStyle penStyle = Qt::DotLine;
    int penWidth = 2;
    //QCPAxis::ScaleType xAxisType = QCPAxis::stLinear;
    //QCPAxis::ScaleType yAxisType = QCPAxis::stLinear;
    QString xAxisLabel = "", yAxisLabel = "";
    bool xAxisTimeFormat = true;
    //QMap<QString, int> lineStyles = QMap<QString, int>{ { "Line", QCPGraph::lsLine }, { "None", QCPGraph::lsNone } };
    //QMap<QString, int> scatterStyles = QMap<QString, int>{ { "None", QCPScatterStyle::ssNone }, { "Cross", QCPScatterStyle::ssCross }, { "Cross Circle", QCPScatterStyle::ssCrossCircle }, { "Dot", QCPScatterStyle::ssDot } };
    //QMap<QString, int> axisTypes = QMap<QString, int>{ { "Normal", QCPAxis::ScaleType::stLinear }, { "Log", QCPAxis::ScaleType::stLogarithmic } };
    //QMap<QString, int> axisTimeFormats = QMap<QString, int>{ { "Number", 0 },{ "Time", 1 } };
    //QMap<QString, int> penStyles = QMap<QString, int>{ { "Solid Line", Qt::SolidLine }, { "Dot Line", Qt::DotLine }, { "Dash Line", Qt::DashLine }, { "Dash Dot Line", Qt::DashDotLine }, { "Dash Dot Dot Line", Qt::DashDotDotLine } };
    //QMap<QString, int> colors = QMap<QString, int> { { "Red", Qt::red }, { "Black", Qt::black }, { "Blue", Qt::blue }, { "Green", Qt::green }, { "Cyan", Qt::cyan }, { "Dark Green", Qt::darkGreen },
    //{ "Dark Red", Qt::darkRed }, { "Dark Blue", Qt::darkBlue }, { "Dark Gray", Qt::darkGray }, { "Magenta", Qt::magenta }, { "Light Gray", Qt::lightGray }, { "Yellow", Qt::yellow } };
    //QMap<QString, int> penWidths = QMap<QString, int>{ { "1", 1 }, { "2", 2 }, { "3", 3 }, { "4", 4 }, { "5", 5 } };
    //QMap<QString, int> legends = QMap<QString, int>{ { "Show", 1 }, { "Hide", 0 } };

};


struct _timeseries
{
    QString filename;
    QString name;
    TimeSeriesSet<outputtimeseriesprecision> Data;
};
*/

namespace Ui {
class QPlotWindow;
}

class QPlotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QPlotWindow(MainWindow *parent = nullptr);
    ~QPlotWindow();
    bool PlotData(const TimeSeries<outputtimeseriesprecision>& BTC, bool allowtime=true, string style="line");
    bool PlotData(const TimeSeriesSet<outputtimeseriesprecision>& BTC, bool allowtime=true, string style="line");
    bool AddData(const TimeSeries<outputtimeseriesprecision>& BTC,bool allowtime=true, string style="line");
    void SetYAxisTitle(const QString& s)
    {
        y_Axis_Title = s;
        if (axisY)
            axisY->setTitleText(s);
    }
    void SetXAxisTitle(const QString& s)
    {
        x_Axis_Title = s;
        if (axisX_date)
            axisX_date->setTitleText(s);
        if (axisX_normal)
            axisX_normal->setTitleText(s);
    }
    void SetLegend(bool val);
    void SetTimeFormat(bool timedate);
    TimeSeries<double> GetTimeSeries(const QString &name) {return timeSeries[name];}
    bool SetYAxis(bool log);
private:
    Ui::QPlotWindow *ui;
    QPlotter* chart = nullptr;
    ChartView *chartview = nullptr;
    double xtoTime(const double &x) {
        return x * 86400 - 2209161600;

    }
    double timetoX(const double &time) {
        return (time + 2209161600) / 86400;
    }
    QDateTime xToDateTime(const double &xltime)
    {
        QDateTime temp_time = QDateTime::fromSecsSinceEpoch(xtoTime(xltime));
        int local_offset = temp_time.offsetFromUtc();
        int fixed_timestamp = xtoTime(xltime) - local_offset;
        QDateTime out = QDateTime::fromSecsSinceEpoch(fixed_timestamp);
        return out;

    }
    QMap<QString,TimeSeries<double>> timeSeries;
    QValueAxis* axisY = nullptr;
    QLogValueAxis* axisY_log = nullptr;
    QValueAxis* axisX_normal = nullptr;
    QDateTimeAxis *axisX_date = nullptr;
    double x_min_val = 1e7;
    double x_max_val = -1e7;
    double y_min_val = 1e7;
    double y_max_val = -1e7;
    QDateTime start;
    QDateTime end;
    QString x_Axis_Title;
    QString y_Axis_Title;


private slots:
     void contextMenuRequest(QPoint pos);
     void ExportToPNG();
     void ExportToCSV();

};

#endif // QPLOTWINDOW_H
