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


#include "qplotwindow.h"
#include "ui_qplotwindow.h"
#include "chartview.h"

#include "mainwindow.h"

QPlotWindow::QPlotWindow(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::QPlotWindow)
{
    ui->setupUi(this);
    chart = new QPlotter();
    chartview = new ChartView(chart, this, parent);
    ui->verticalLayout->addWidget(chartview);
    qDebug()<<parent->resource_directory;
    QIcon icon_png = QIcon(parent->resource_directory + "/Icons/Graph.png");
    QIcon icon_csv = QIcon(parent->resource_directory + "/Icons/export.png");
    connect(ui->ExportToPNG,SIGNAL(clicked()),this, SLOT(ExportToPNG()));
    ui->ExportToPNG->setIcon(icon_png);
    connect(ui->ExporttoCSV,SIGNAL(clicked()),this, SLOT(ExportToCSV()));
    ui->ExporttoCSV->setIcon(icon_csv);
    //chart->setContextMenuPolicy(Qt::CustomContextMenu);

}

QPlotWindow::~QPlotWindow()
{
    delete ui;
}


bool QPlotWindow::PlotData(const TimeSeries<outputtimeseriesprecision>& timeseries, bool allowtime, string style)
{
    x_min_val = timeseries.mint();
    x_max_val = timeseries.maxt();
    y_min_val = timeseries.minC();
    y_max_val = timeseries.maxC();
    if (y_min_val==y_max_val)
    {
        y_min_val*=0.8;
        y_max_val*=1.2;
        if (y_max_val==0)
            y_max_val=1.0;
    }
    if (x_max_val<20000)
        allowtime = false;
#ifndef Qt6
    //QDateTime _date = QDateTime::fromSecsSinceEpoch(xtoTime(25569),QTimeZone::utc());
    //QDateTime _date2 = QDateTime::fromSecsSinceEpoch(0);
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#else
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#endif

    QString xAxisTitle = x_Axis_Title;
    QString yAxisTitle = y_Axis_Title;
    axisX_normal = new QValueAxis();
    axisX_date = new QDateTimeAxis();

    axisX_normal->setTickCount(10);
    axisX_date->setTickCount(10);
    if (!allowtime)
    {   axisX_normal->setTitleText("X");
        chart->addAxis(axisX_normal, Qt::AlignBottom);
        axisX_normal->setObjectName("axisX_normal");
        axisX_normal->setTitleText(xAxisTitle);
        axisX_normal->setRange(x_min_val ,x_max_val);
    }
    else
    {
        axisX_date->setTitleText("X");
        chart->addAxis(axisX_date, Qt::AlignBottom);
        axisX_date->setObjectName("axisX_date");
        axisX_date->setTitleText(xAxisTitle);
        axisX_date->setRange(start ,end);
    }

    if (!chartview->Ylog())
    {
        axisY = new QValueAxis();
        axisY->setObjectName("axisY_normal");
        axisY->setTitleText(yAxisTitle);
        axisY->setRange(y_min_val,y_max_val);
        chart->addAxis(axisY, Qt::AlignLeft);
    }
    else
    {
        axisY_log = new QLogValueAxis();
        axisY_log->setObjectName("axisY_log");
        axisY_log->setTitleText(yAxisTitle);
        axisY_log->setLabelFormat("e");
        axisY_log->setRange(y_min_val,y_max_val);
        axisY_log->setMinorTickCount(8);
        chart->addAxis(axisY_log, Qt::AlignLeft);
    }


    QLineSeries *lineseries = new QLineSeries();

    for (int j=0; j<timeseries.size(); j++)
    {
        if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(timeseries.getTime(j)).toMSecsSinceEpoch(),timeseries.getValue(j));
#else
            lineseries->append(xToDateTime(timeseries.getTime(j)).toMSecsSinceEpoch(),timeseries.getValue(j));
#endif
        else
            lineseries->append(timeseries.getTime(j),timeseries.getValue(j));
    }
    chart->addSeries(lineseries);

    if (allowtime)
        lineseries->attachAxis(axisX_date);
    else
        lineseries->attachAxis(axisX_normal);
    if (!chartview->Ylog())
        lineseries->attachAxis(axisY);
    else
        lineseries->attachAxis(axisY_log);

    QPen pen = lineseries->pen();
    pen.setWidth(2);
    pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
    lineseries->setPen(pen);
    lineseries->setName(QString::fromStdString(timeseries.name()));
    timeSeries.insert(lineseries->name(),timeseries);



    if (allowtime)
    {   if (start.secsTo(end) < 600) {axisX_date->setFormat("mm:ss:zzz"); axisX_date->setTitleText("Time");}
        if (start.secsTo(end) > 3600) {axisX_date->setFormat("hh:mm:ss"); axisX_date->setTitleText("Time");}
        if (start.daysTo(end) > 1) {axisX_date->setFormat("MMM dd\nhh:mm:ss"); axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 5) {axisX_date->setFormat("MM.dd.yyyy\nhh:mm");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 180) {axisX_date->setFormat("MM.dd.yyyy\nhAP");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 2 * 365) {axisX_date->setFormat("MMMM\nyyyy");axisX_date->setTitleText("Date");}
        axisX_date->setLabelsAngle(-90);
    }
    else
        axisX_normal->setLabelsAngle(-90);
    return true;
}
bool QPlotWindow::PlotData(const TimeSeriesSet<outputtimeseriesprecision>& timeseriesset, bool allowtime, string style)
{
    x_min_val = timeseriesset.mintime();
    x_max_val = timeseriesset.maxtime();
    y_min_val = timeseriesset.minval();
    y_max_val = timeseriesset.maxval();
    if (y_min_val==y_max_val)
    {
        y_min_val*=0.8;
        y_max_val*=1.2;
        if (y_max_val==0)
            y_max_val=1.0;
    }
    if (x_max_val<20000)
        allowtime = false;

#ifndef Qt6
        start = xToDateTime(x_min_val);
        end = xToDateTime(x_max_val);
#else
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#endif

    QString xAxisTitle = x_Axis_Title;
    QString yAxisTitle = y_Axis_Title;
    axisX_normal = new QValueAxis();
    axisX_date = new QDateTimeAxis;

    axisX_normal->setTickCount(10);
    axisX_date->setTickCount(10);
    if (!allowtime)
    {   axisX_normal->setTitleText(x_Axis_Title);
        chart->addAxis(axisX_normal, Qt::AlignBottom);
        axisX_normal->setObjectName("axisX");
        axisX_normal->setTitleText(xAxisTitle);
        axisX_normal->setRange(x_min_val ,x_max_val);
    }
    else
    {
        axisX_date->setTitleText(x_Axis_Title);
        chart->addAxis(axisX_date, Qt::AlignBottom);
        axisX_date->setObjectName("axisX");
        axisX_date->setTitleText(xAxisTitle);
        axisX_date->setRange(start ,end);
    }

    if (!chartview->Ylog())
    {
        axisY = new QValueAxis();
        axisY->setObjectName("axisY");
        axisY->setTitleText(yAxisTitle);
        axisY->setRange(y_min_val,y_max_val);
        chart->addAxis(axisY, Qt::AlignLeft);
    }
    else
    {
        axisY_log = new QLogValueAxis();
        axisY_log->setObjectName("axisY");
        axisY_log->setTitleText(yAxisTitle);
        axisY_log->setRange(y_min_val,y_max_val);
        axisY_log->setMinorTickCount(8);
        chart->addAxis(axisY_log, Qt::AlignLeft);
    }
    for (int i=0; i<timeseriesset.size(); i++)
    {   QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        if (allowtime)
            lineseries->attachAxis(axisX_date);
        else
            lineseries->attachAxis(axisX_normal);
        if (!chartview->Ylog())
            lineseries->attachAxis(axisY);
        else
            lineseries->attachAxis(axisY_log);

        for (int j=0; j<timeseriesset[i].size(); j++)
        {
            if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(timeseriesset[i].getTime(j)).toMSecsSinceEpoch(),timeseriesset[i].getValue(j));
#else
            lineseries->append(xToDateTime(timeseriesset[i].getTime(j)).toMSecsSinceEpoch(),timeseriesset[i].getValue(j));
#endif
            else
                lineseries->append(timeseriesset[i].getTime(j),timeseriesset[i].getValue(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset.getSeriesName(i)));
        timeSeries.insert(lineseries->name(),timeseriesset[i]);

    }



    if (allowtime)
    {   if (start.secsTo(end) < 600) {axisX_date->setFormat("mm:ss:zzz"); axisX_date->setTitleText("Time");}
        if (start.secsTo(end) > 3600) {axisX_date->setFormat("hh:mm:ss"); axisX_date->setTitleText("Time");}
        if (start.daysTo(end) > 1) {axisX_date->setFormat("MMM dd\nhh:mm:ss"); axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 5) {axisX_date->setFormat("MM.dd.yyyy\nhh:mm");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 180) {axisX_date->setFormat("MM.dd.yyyy\nhAP");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 2 * 365) {axisX_date->setFormat("MMMM\nyyyy");axisX_date->setTitleText("Date");}
        axisX_date->setLabelsAngle(-90);
    }
    else
        axisX_normal->setLabelsAngle(-90);
    return true;
}
bool QPlotWindow::AddData(const TimeSeries<outputtimeseriesprecision>& timeseries,bool allowtime, string style)
{
    x_min_val = min(timeseries.mint(),x_min_val);
    x_max_val = max(timeseries.maxt(),x_max_val);
    y_min_val = min(timeseries.minC(),y_min_val);
    y_max_val = max(timeseries.maxC(),y_max_val);
    if (y_min_val==y_max_val)
    {
        y_min_val*=0.8;
        y_max_val*=1.2;
        if (y_max_val==0)
            y_max_val=1.0;
    }

    if (x_max_val<20000)
        allowtime = false;
#ifndef Qt6
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#else
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#endif

    QLineSeries *lineseries = new QLineSeries();
    lineseries->setName(QString::fromStdString(timeseries.name()));
    chart->addSeries(lineseries);
    if (allowtime)
    {   lineseries->attachAxis(axisX_date);
        axisX_date->setRange(start ,end);
    }
    else
    {   lineseries->attachAxis(axisX_normal);
        axisX_normal->setRange(x_min_val ,x_max_val);
    }

    if (!chartview->Ylog())
    {
        lineseries->attachAxis(axisY);
        axisY->setRange(y_min_val,y_max_val);
    }
    else
    {
        lineseries->attachAxis(axisY_log);
        axisY->setRange(max(y_min_val,1e-6),max(y_max_val,1e-6));
    }
    QVector<QPointF> points;
    points.reserve(timeseries.size());

    for (int j = 0; j < timeseries.size(); ++j)
    {
        double x = allowtime
            ? xToDateTime(timeseries.getTime(j)).toMSecsSinceEpoch()
            : timeseries.getTime(j);
        double y = timeseries.getValue(j);
        points.append(QPointF(x, y));
    }

    lineseries->append(points);

    QPen pen = lineseries->pen();
    pen.setWidth(2);
    pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
    lineseries->setPen(pen);
    timeSeries.insert(lineseries->name(),timeseries);

    return true;
}

void QPlotWindow::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec();

}

void QPlotWindow::SetLegend(bool val)
{
    chart->legend()->setVisible(val);
}

bool QPlotWindow::SetYAxis(bool log)
{

    if (!chartview->Ylog())
    {
        axisY = new QValueAxis();
        axisY->setObjectName("axisY");
        axisY->setTitleText(x_Axis_Title);
        axisY->setRange(y_min_val,y_max_val);

        chart->addAxis(axisY, Qt::AlignLeft);
        chart->removeAxis(axisY_log);
        for (int i=0; i<chart->series().size(); i++)
        {
            chart->series()[i]->attachAxis(axisY);
        }
    }
    else
    {
        axisY_log = new QLogValueAxis();
        axisY_log->setObjectName("axisY");
        axisY_log->setTitleText(y_Axis_Title);
        axisY_log->setRange(max(y_min_val,1e-6),y_max_val);
        axisY_log->setLabelFormat("%.0e");

        axisY_log->setMinorTickCount(8);
        chart->addAxis(axisY_log, Qt::AlignLeft);
        chart->removeAxis(axisY);
        for (int i=0; i<chart->series().size(); i++)
        {
            chart->series()[i]->attachAxis(axisY_log);
        }
    }

    return true; 

}

void QPlotWindow::ExportToPNG()
{
    QRect rect = QDialog::frameGeometry();

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("png file (*.png)"));

    if (!fileName.contains("."))
        fileName+=".png";
    chart->setAnimationOptions(QChart::NoAnimation);
    this->resize(rect.width() * 2, rect.height() * 2);

    chartview->grab().save(fileName);
    this->resize(rect.size());
}

void QPlotWindow::ExportToCSV()
{
    QRect rect = QDialog::frameGeometry();

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("csv file (*.csv)"));

    if (!fileName.contains("."))
        fileName+=".csv";

    TimeSeriesSet<double> towrite;
    for (QMap<QString, TimeSeries<double>>::iterator it = timeSeries.begin(); it!=timeSeries.end(); it++)
        towrite.append(it.value(),it.key().toStdString());

    towrite.write(fileName.toStdString());
}
