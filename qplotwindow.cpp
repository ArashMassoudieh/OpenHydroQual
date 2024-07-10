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


bool QPlotWindow::PlotData(const CTimeSeries<outputtimeseriesprecision>& timeseries, bool allowtime, string style)
{
    x_min_val = timeseries.mint();
    x_max_val = timeseries.maxt();
    y_min_val = timeseries.minC();
    y_max_val = timeseries.maxC();
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

    for (int j=0; j<timeseries.n; j++)
    {
        if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(timeseries.GetT(j)).toMSecsSinceEpoch(),timeseries.GetC(j));
#else
            lineseries->append(xToDateTime(timeseries.GetT(j)).toMSecsSinceEpoch(),timeseries.GetC(j));
#endif
        else
            lineseries->append(timeseries.GetT(j),timeseries.GetC(j));
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
    lineseries->setName(QString::fromStdString(timeseries.name));
    TimeSeries.insert(lineseries->name(),timeseries);



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
bool QPlotWindow::PlotData(const CTimeSeriesSet<outputtimeseriesprecision>& timeseriesset, bool allowtime, string style)
{
    x_min_val = timeseriesset.mintime();
    x_max_val = timeseriesset.maxtime();
    y_min_val = timeseriesset.minval();
    y_max_val = timeseriesset.maxval();

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
    for (int i=0; i<timeseriesset.nvars; i++)
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

        for (int j=0; j<timeseriesset.BTC[i].n; j++)
        {
            if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(timeseriesset.BTC[i].GetT(j)).toMSecsSinceEpoch(),timeseriesset.BTC[i].GetC(j));
#else
            lineseries->append(xToDateTime(timeseriesset.BTC[i].GetT(j)).toMSecsSinceEpoch(),timeseriesset.BTC[i].GetC(j));
#endif
            else
                lineseries->append(timeseriesset.BTC[i].GetT(j),timeseriesset.BTC[i].GetC(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset.names[i]));
        TimeSeries.insert(lineseries->name(),timeseriesset.BTC[i]);

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
bool QPlotWindow::AddData(const CTimeSeries<outputtimeseriesprecision>& timeseries,bool allowtime, string style)
{
    x_min_val = min(timeseries.mint(),x_min_val);
    x_max_val = max(timeseries.maxt(),x_max_val);
    y_min_val = min(timeseries.minC(),y_min_val);
    y_max_val = max(timeseries.maxC(),y_max_val);

#ifndef Qt6
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#else
    start = xToDateTime(x_min_val);
    end = xToDateTime(x_max_val);
#endif

    QLineSeries *lineseries = new QLineSeries();
    lineseries->setName(QString::fromStdString(timeseries.name));
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
    for (int j=0; j<timeseries.n; j++)
    {
        if (allowtime)
#ifndef Qt6
            lineseries->append(xToDateTime(timeseries.GetT(j)).toMSecsSinceEpoch(),timeseries.GetC(j));
#else
            lineseries->append(xToDateTime(timeseries.GetT(j)).toMSecsSinceEpoch(),timeseries.GetC(j));
#endif
        else
            lineseries->append(timeseries.GetT(j),timeseries.GetC(j));
    }


    QPen pen = lineseries->pen();
    pen.setWidth(2);
    pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
    lineseries->setPen(pen);
    TimeSeries.insert(lineseries->name(),timeseries);

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

    CTimeSeriesSet<double> towrite;
    for (QMap<QString, CTimeSeries<double>>::iterator it = TimeSeries.begin(); it!=TimeSeries.end(); it++)
        towrite.append(it.value(),it.key().toStdString());

    towrite.writetofile(fileName.toStdString());
}
