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
#include "XString.h"

#include "mainwindow.h"

QPlotWindow::QPlotWindow(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::QPlotWindow)
{
    ui->setupUi(this);
    // Add after ui->setupUi(this); in the constructor:
    datasetSelector = new QComboBox(this);
    displayModeSelector = new QComboBox(this);
    displayModeSelector->addItem("Lines");
    displayModeSelector->addItem("Symbols");

    QLabel* datasetLabel = new QLabel("Dataset:", this);
    QLabel* modeLabel = new QLabel("Display:", this);

    // Add these to your layout - you'll need to modify the layout structure
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->addWidget(datasetLabel);
    controlLayout->addWidget(datasetSelector);
    controlLayout->addWidget(modeLabel);
    controlLayout->addWidget(displayModeSelector);
    ui->verticalLayout->insertLayout(0, controlLayout);

    connect(datasetSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(onDatasetSelected(int)));
    connect(displayModeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayModeChanged(int)));
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
    qDebug() << "=== PlotData (TimeSeries) called ===";
    qDebug() << "  TimeSeries size:" << timeseries.size();

    // === STORE ORIGINAL SI DATA ===
    if (quantity && !original_data_stored)  // CHANGED: use flag instead of size check
    {
        qDebug() << "  Storing original SI data (TimeSeries as TimeSeriesSet)";
        // Convert TimeSeries to TimeSeriesSet for storage
        original_timeseriesset_SI = TimeSeriesSet<double>();
        original_timeseriesset_SI.append(timeseries, timeseries.name());
        original_data_stored = true;  // SET FLAG
        qDebug() << "  Stored, original_timeseriesset_SI.size():" << original_timeseriesset_SI.size();
    }

    // === CONVERT DATA IF NEEDED ===
    TimeSeries<outputtimeseriesprecision> data_to_plot = timeseries;

    if (quantity && !current_display_unit.isEmpty())
    {
        QString default_unit = XString::reform(QString::fromStdString(quantity->DefaultUnit()));

        qDebug() << "  Quantity set:";
        qDebug() << "    Default unit (SI):" << default_unit;
        qDebug() << "    Display unit:" << current_display_unit;

        if (current_display_unit != default_unit)
        {
            qDebug() << "    Converting data";

            double si_coeff = XString::coefficient(XString::reformBack(default_unit));
            double display_coeff = XString::coefficient(XString::reformBack(current_display_unit));

            qDebug() << "    SI coeff:" << si_coeff;
            qDebug() << "    Display coeff:" << display_coeff;

            if (si_coeff != 0 && display_coeff != 0)
            {
                double conversion_factor = si_coeff / display_coeff;
                qDebug() << "    Conversion factor:" << conversion_factor;

                // Convert single TimeSeries
                data_to_plot = TimeSeries<outputtimeseriesprecision>();
                data_to_plot.reserve(timeseries.size());
                for (const auto& pt : timeseries)
                {
                    data_to_plot.addPoint(pt.t, pt.c * conversion_factor, pt.d);
                }
                data_to_plot.setStructured(timeseries.isStructured());
                data_to_plot.setName(timeseries.name());

                qDebug() << "    Converted, max value:" << data_to_plot.maxC();
            }
        }

        // Update y-axis title
        QString base_title = y_Axis_Title;
        if (base_title.contains('['))
        {
            base_title = base_title.left(base_title.indexOf('[')).trimmed();
        }
        y_Axis_Title = base_title + " [" + current_display_unit + "]";
        qDebug() << "    Y-axis title:" << y_Axis_Title;
    }

    x_min_val = data_to_plot.mint();
    x_max_val = data_to_plot.maxt();
    y_min_val = data_to_plot.minC();
    y_max_val = data_to_plot.maxC();
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
    axisX_date = new QDateTimeAxis();

    axisX_normal->setTickCount(10);
    axisX_date->setTickCount(10);
    if (!allowtime)
    {
        axisX_normal->setTitleText("X");
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

    for (int j=0; j<data_to_plot.size(); j++)  // CHANGED
    {
        if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(data_to_plot.getTime(j)).toMSecsSinceEpoch(),data_to_plot.getValue(j));
#else
            lineseries->append(xToDateTime(data_to_plot.getTime(j)).toMSecsSinceEpoch(),data_to_plot.getValue(j));
#endif
        else
            lineseries->append(data_to_plot.getTime(j),data_to_plot.getValue(j));
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
    QString seriesName = QString::fromStdString(data_to_plot.name());  // CHANGED
    if (seriesName.isEmpty()) {
        seriesName = QString("Series %1").arg(unnamed_series_counter++);
    }
    lineseries->setName(seriesName);
    timeSeries.insert(lineseries->name(),data_to_plot);  // CHANGED

    if (allowtime)
    {
        if (start.secsTo(end) < 600) {axisX_date->setFormat("mm:ss:zzz"); axisX_date->setTitleText("Time");}
        if (start.secsTo(end) > 3600) {axisX_date->setFormat("hh:mm:ss"); axisX_date->setTitleText("Time");}
        if (start.daysTo(end) > 1) {axisX_date->setFormat("MMM dd\nhh:mm:ss"); axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 5) {axisX_date->setFormat("MM.dd.yyyy\nhh:mm");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 180) {axisX_date->setFormat("MM.dd.yyyy\nhAP");axisX_date->setTitleText("Date");}
        if (start.daysTo(end) > 2 * 365) {axisX_date->setFormat("MMMM\nyyyy");axisX_date->setTitleText("Date");}
        axisX_date->setLabelsAngle(-90);
    }
    else
        axisX_normal->setLabelsAngle(-90);

    seriesDisplayMode.insert(lineseries->name(), "line");
    if (datasetSelector->findText(lineseries->name()) == -1) {
        datasetSelector->addItem(lineseries->name());
    }

    qDebug() << "=== PlotData (TimeSeries) complete ===";
    return true;
}

bool QPlotWindow::PlotData(const TimeSeriesSet<outputtimeseriesprecision>& timeseriesset, bool allowtime, string style)
{
    qDebug() << "=== PlotData (TimeSeriesSet) called ===";
    qDebug() << "  TimeSeriesSet size:" << timeseriesset.size();

    // === HIDE UNIT DROPDOWN IF MORE THAN ONE SERIES ===
    if (timeseriesset.size() > 1 && unitSelector)
    {
        qDebug() << "  Multiple series detected, hiding unit dropdown";
        unitSelector->setVisible(false);

        // Hide the label too
        QWidget* parentWidget = qobject_cast<QWidget*>(unitSelector->parent());
        if (parentWidget && parentWidget->layout())
        {
            QLayout* layout = parentWidget->layout();
            for (int i = 0; i < layout->count(); i++)
            {
                QWidget* widget = layout->itemAt(i)->widget();
                if (QLabel* label = qobject_cast<QLabel*>(widget))
                {
                    if (label->text() == "Unit:")
                        label->setVisible(false);
                }
            }
        }
    }

    // === STORE ORIGINAL SI DATA (only ONCE and only for single series) ===
    if (quantity && !original_data_stored && timeseriesset.size() == 1)  // CHANGED: use flag
    {
        qDebug() << "  Storing original SI data";
        original_timeseriesset_SI = timeseriesset;
        original_data_stored = true;  // SET FLAG
        qDebug() << "  Stored, flag set to true";
    }

    // === CONVERT DATA IF NEEDED (only for single series) ===
    TimeSeriesSet<outputtimeseriesprecision> data_to_plot = timeseriesset;

    if (quantity && !current_display_unit.isEmpty() && timeseriesset.size() == 1)
    {
        QString default_unit = XString::reform(QString::fromStdString(quantity->DefaultUnit()));

        qDebug() << "  Quantity set:";
        qDebug() << "    Default unit (SI):" << default_unit;
        qDebug() << "    Display unit:" << current_display_unit;

        if (current_display_unit != default_unit)
        {
            qDebug() << "    Converting data";

            double si_coeff = XString::coefficient(XString::reformBack(default_unit));
            double display_coeff = XString::coefficient(XString::reformBack(current_display_unit));

            qDebug() << "    SI coeff:" << si_coeff;
            qDebug() << "    Display coeff:" << display_coeff;

            if (si_coeff != 0 && display_coeff != 0)
            {
                double conversion_factor = si_coeff /display_coeff;
                qDebug() << "    Conversion factor:" << conversion_factor;

                // Convert the TimeSeriesSet
                data_to_plot = timeseriesset * conversion_factor;

                qDebug() << "    Converted, max value:" << data_to_plot[0].maxC();
            }
        }

        // Update y-axis title with unit
        QString base_title = y_Axis_Title;
        if (base_title.contains('['))
        {
            base_title = base_title.left(base_title.indexOf('[')).trimmed();
        }
        y_Axis_Title = base_title + " [" + current_display_unit + "]";
        qDebug() << "    Y-axis title:" << y_Axis_Title;
    }

    x_min_val = data_to_plot.mintime();
    x_max_val = data_to_plot.maxtime();
    y_min_val = data_to_plot.minval();
    y_max_val = data_to_plot.maxval();

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
    {
        axisX_normal->setTitleText(x_Axis_Title);
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

    for (int i=0; i<data_to_plot.size(); i++)  // CHANGED: use data_to_plot
    {
        QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        if (allowtime)
            lineseries->attachAxis(axisX_date);
        else
            lineseries->attachAxis(axisX_normal);
        if (!chartview->Ylog())
            lineseries->attachAxis(axisY);
        else
            lineseries->attachAxis(axisY_log);

        for (int j=0; j<data_to_plot[i].size(); j++)  // CHANGED: use data_to_plot
        {
            if (allowtime)
#ifndef Qt6
                lineseries->append(xToDateTime(data_to_plot[i].getTime(j)).toMSecsSinceEpoch(),data_to_plot[i].getValue(j));
#else
                lineseries->append(xToDateTime(data_to_plot[i].getTime(j)).toMSecsSinceEpoch(),data_to_plot[i].getValue(j));
#endif
            else
                lineseries->append(data_to_plot[i].getTime(j),data_to_plot[i].getValue(j));
        }

        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(data_to_plot.getSeriesName(i)));  // CHANGED: use data_to_plot
        timeSeries.insert(lineseries->name(),data_to_plot[i]);  // CHANGED: use data_to_plot

        seriesDisplayMode.insert(lineseries->name(), "line");
        if (datasetSelector->findText(lineseries->name()) == -1) {
            datasetSelector->addItem(lineseries->name());
        }
    }

    if (allowtime)
    {
        if (start.secsTo(end) < 600) {axisX_date->setFormat("mm:ss:zzz"); axisX_date->setTitleText("Time");}
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
    QString seriesName = QString::fromStdString(timeseries.name());
    if (seriesName.isEmpty()) {
        seriesName = QString("Series %1").arg(unnamed_series_counter++);
    }
    lineseries->setName(seriesName);
    seriesDisplayMode.insert(lineseries->name(), "line");
    datasetSelector->addItem(lineseries->name());
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

    lineseries->append(points.toList());

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

void QPlotWindow::onDatasetSelected(int index)
{
    if (index < 0) return;
    QString selectedName = datasetSelector->currentText();
    QString currentMode = seriesDisplayMode.value(selectedName, "line");
    displayModeSelector->blockSignals(true);
    displayModeSelector->setCurrentText(currentMode == "line" ? "Lines" : "Symbols");
    displayModeSelector->blockSignals(false);
}

void QPlotWindow::onDisplayModeChanged(int index)
{
    QString selectedName = datasetSelector->currentText();
    if (selectedName.isEmpty()) return;

    QString mode = (displayModeSelector->currentIndex() == 0) ? "line" : "symbols";
    updateSeriesDisplay(selectedName, mode);
}

void QPlotWindow::updateSeriesDisplay(const QString& seriesName, const QString& mode)
{
    // Find the series
    QAbstractSeries* oldSeries = nullptr;
    foreach (QAbstractSeries* series, chart->series()) {
        if (series->name() == seriesName) {
            oldSeries = series;
            break;
        }
    }
    if (!oldSeries) return;

    // Get the timeseries data
    TimeSeries<double> ts = timeSeries[seriesName];

    // Save the pen/color and get attached axes BEFORE removing
    QPen oldPen;
    QList<QAbstractAxis*> attachedAxes;

    if (QLineSeries* lineSeries = qobject_cast<QLineSeries*>(oldSeries)) {
        oldPen = lineSeries->pen();
        attachedAxes = lineSeries->attachedAxes();
    } else if (QScatterSeries* scatterSeries = qobject_cast<QScatterSeries*>(oldSeries)) {
        oldPen = scatterSeries->pen();
        oldPen.setWidth(2); // Reset width for line series
        attachedAxes = scatterSeries->attachedAxes();
    }

    // Determine if time axis is being used
    bool allowtime = false;
    QAbstractAxis* xAxisToUse = nullptr;
    QAbstractAxis* yAxisToUse = nullptr;

    foreach (QAbstractAxis* axis, attachedAxes) {
        if (axis->alignment() == Qt::AlignBottom) {
            xAxisToUse = axis;
            if (qobject_cast<QDateTimeAxis*>(axis)) {
                allowtime = true;
            }
        } else if (axis->alignment() == Qt::AlignLeft) {
            yAxisToUse = axis;
        }
    }

    // Remove old series
    chart->removeSeries(oldSeries);
    oldSeries->deleteLater();

    // Create new series based on mode
    QAbstractSeries* newSeries = nullptr;

    if (mode == "symbols") {
        QScatterSeries* scatterSeries = new QScatterSeries();
        scatterSeries->setName(seriesName);
        scatterSeries->setMarkerSize(10.0);

        for (int j = 0; j < ts.size(); ++j) {
            double x = allowtime ? xToDateTime(ts.getTime(j)).toMSecsSinceEpoch() : ts.getTime(j);
            scatterSeries->append(x, ts.getValue(j));
        }

        scatterSeries->setPen(oldPen);
        scatterSeries->setBrush(oldPen.brush());
        scatterSeries->setColor(oldPen.color());

        newSeries = scatterSeries;

    } else {
        QLineSeries* lineSeries = new QLineSeries();
        lineSeries->setName(seriesName);

        for (int j = 0; j < ts.size(); ++j) {
            double x = allowtime ? xToDateTime(ts.getTime(j)).toMSecsSinceEpoch() : ts.getTime(j);
            lineSeries->append(x, ts.getValue(j));
        }

        lineSeries->setPen(oldPen);

        newSeries = lineSeries;
    }

    // Add series to chart
    chart->addSeries(newSeries);

    // Attach the same axes that were attached to the old series
    if (xAxisToUse) {
        newSeries->attachAxis(xAxisToUse);
    }
    if (yAxisToUse) {
        newSeries->attachAxis(yAxisToUse);
    }

    seriesDisplayMode[seriesName] = mode;
}

void QPlotWindow::SetQuantity(Quan* quan)
{
    quantity = quan;

    if (!quan)
        return;

    QString units_str = QString::fromStdString(quan->Units());

    if (units_str.isEmpty())
    {
        qDebug() << "Quan has no units, skipping unit dropdown";
        return;
    }

    QStringList unitsList = units_str.split(';', Qt::SkipEmptyParts);

    if (unitsList.isEmpty())
        return;

    unitSelector = new QComboBox(this);

    // Reform each unit for display - CONVERT ^ to ~^ first
    QStringList reformedUnits;
    for (QString unit : unitsList)
    {
        // Replace ^ with ~^ so XString::reform() can convert to superscript
        if (!unit.contains("~^"))
            unit.replace("^", "~^");
        reformedUnits << XString::reform(unit);
    }
    unitSelector->addItems(reformedUnits);

    // Set current unit to the Quan's current unit (or default unit)
    QString currentUnit = QString::fromStdString(quan->Unit());
    if (currentUnit.isEmpty())
        currentUnit = QString::fromStdString(quan->DefaultUnit());

    // Apply XString::reform to display properly
    currentUnit = XString::reform(currentUnit);

    int index = unitSelector->findText(currentUnit);
    if (index >= 0)
        unitSelector->setCurrentIndex(index);
    else
        unitSelector->setCurrentIndex(0);  // Fallback to first unit

    current_display_unit = unitSelector->currentText();

    // Create a horizontal layout for the unit selector
    QHBoxLayout* unitLayout = new QHBoxLayout();
    QLabel* unitLabel = new QLabel("Unit:", this);
    unitLayout->addWidget(unitLabel);
    unitLayout->addWidget(unitSelector);
    unitLayout->addStretch();  // Push everything to the left

    // Insert the unit layout into the existing horizontalLayout (with export buttons)
    // This way it appears on the same row as the export buttons
    ui->horizontalLayout->insertWidget(0, unitLabel);
    ui->horizontalLayout->insertWidget(1, unitSelector);

    // Connect the signal
    connect(unitSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QPlotWindow::onUnitChanged);

    qDebug() << "SetQuantity: Units available:" << reformedUnits;
    qDebug() << "SetQuantity: Current unit:" << current_display_unit;
}
void QPlotWindow::convertAndUpdatePlot(const QString& new_unit)
{
    qDebug() << "=== convertAndUpdatePlot called ===";
    qDebug() << "=== convertAndUpdatePlot called ===";
    qDebug() << "  new_unit parameter:" << new_unit;
    qDebug() << "  new_unit contains ~:" << new_unit.contains("~");

    if (!quantity || original_timeseriesset_SI.size() == 0)
        return;

    QString default_unit = XString::reform(QString::fromStdString(quantity->DefaultUnit()));
    double si_coeff = XString::coefficient(XString::reformBack(default_unit));
    double display_coeff = XString::coefficient(XString::reformBack(new_unit));

    if (si_coeff == 0 || display_coeff == 0)
        return;

    double conversion_factor = si_coeff / display_coeff;

    // === PERFORMANCE OPTIMIZATIONS ===

    // 1. Disable animations
    QChart::AnimationOptions oldAnimations = chart->animationOptions();
    chart->setAnimationOptions(QChart::NoAnimation);

    // 2. Block chart updates
    chart->blockSignals(true);

    // 3. Hide chart view during update (prevents redraws)
    if (chartview)
        chartview->setUpdatesEnabled(false);

    QList<QAbstractSeries*> seriesList = chart->series();
    if (!seriesList.isEmpty())
    {
        QLineSeries* lineSeries = qobject_cast<QLineSeries*>(seriesList.first());
        if (lineSeries)
        {
            // Block series signals too
            lineSeries->blockSignals(true);

            // Build converted data
            QVector<QPointF> convertedPoints;
            convertedPoints.reserve(original_timeseriesset_SI[0].size());

            bool allowtime = (original_timeseriesset_SI[0].maxt() > 20000);
            double new_y_min = 1e7;
            double new_y_max = -1e7;

            for (int i = 0; i < original_timeseriesset_SI[0].size(); i++)
            {
                double t = original_timeseriesset_SI[0].getTime(i);
                double convertedValue = original_timeseriesset_SI[0].getValue(i) * conversion_factor;
                double x = allowtime ? xToDateTime(t).toMSecsSinceEpoch() : t;

                convertedPoints.append(QPointF(x, convertedValue));

                if (convertedValue < new_y_min) new_y_min = convertedValue;
                if (convertedValue > new_y_max) new_y_max = convertedValue;
            }

            // Update data
            lineSeries->replace(convertedPoints);
            lineSeries->blockSignals(false);

            // Update Y-axis
            if (new_y_min == new_y_max)
            {
                new_y_min *= 0.8;
                new_y_max *= 1.2;
                if (new_y_max == 0) new_y_max = 1.0;
            }

            if (axisY)
                axisY->setRange(new_y_min, new_y_max);
            else if (axisY_log)
                axisY_log->setRange(qMax(new_y_min, 1e-6), qMax(new_y_max, 1e-6));
        }
    }

    // Update title
    QString base_title = y_Axis_Title;
    if (base_title.contains('['))
        base_title = base_title.left(base_title.indexOf('[')).trimmed();
    QString new_title = base_title + " [" + new_unit + "]";

    if (axisY)
        axisY->setTitleText(new_title);
    else if (axisY_log)
        axisY_log->setTitleText(new_title);

    y_Axis_Title = new_title;
    current_display_unit = new_unit;

    // === RE-ENABLE UPDATES ===
    chart->blockSignals(false);
    if (chartview)
        chartview->setUpdatesEnabled(true);
    chart->setAnimationOptions(oldAnimations);

    qDebug() << "=== convertAndUpdatePlot complete ===";
}

void QPlotWindow::onUnitChanged(int index)
{
    if (!unitSelector || !quantity)
        return;

    QString new_unit = unitSelector->currentText();

    qDebug() << "=== Unit changed ===";
    qDebug() << "  Old unit:" << current_display_unit;
    qDebug() << "  New unit:" << new_unit;

    if (new_unit == current_display_unit)
    {
        qDebug() << "  Units are the same, skipping conversion";
        return;
    }

    // Perform the conversion and update the plot
    convertAndUpdatePlot(new_unit);

    qDebug() << "=== Unit change complete ===";
}
