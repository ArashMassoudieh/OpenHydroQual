#include "plotter.h"
#include "ui_plotter.h"
#include "qcustomplot.h"
#include "qdebug.h"
#include "qfileinfo.h"
#include "qaction.h"
#include "mainwindow.h"



Plotter::Plotter(MainWindow *_parent) :
    QMainWindow(_parent),
    ui(new Ui::Plotter)
{
    parent = _parent;
    ui->setupUi(this);
    plot = new CustomPlotZoom(ui->centralWidget);
    plot->setObjectName(QStringLiteral("customPlot"));
    ui->horizontalLayout->addWidget(plot);
    QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(2);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(plot->sizePolicy().hasHeightForWidth());
    plot->setSizePolicy(sizePolicy2);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
    connect(ui->actionLegend, SIGNAL(triggered()), this, SLOT(On_Legend_Clicked()));
    connect(plot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(plot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    connect(plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(plot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(plot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    plot->axisRect()->setupFullAxesBox();
    connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
    connect(plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
    connect(plot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(Deselect()));

}

Plotter::~Plotter()
{
    delete ui;
}



bool Plotter::PlotData(const CTimeSeriesSet<timeseriesprecision>& BTC, bool allowtime, string style)
{
    qDebug()<<"plot 0";
    qDebug()<<"nvars = " << BTC.nvars;
    PlotData(BTC[0],allowtime);

    for (unsigned int i=1; i<BTC.nvars; i++)
    {
        qDebug()<<"plot " << i;
        AddData(BTC[i],allowtime);
    }
    qDebug()<<"All plots added";
    return true;
}

bool Plotter::PlotData(const CTimeSeries<timeseriesprecision>& BTC, bool allowtime, string style)
{
    minx = min(BTC.GetT(0),minx);
    maxy = max(BTC.maxC()*(1+0.1*sgn(BTC.maxC())),maxy);
    maxx = max(BTC.GetT(BTC.n-1),maxx);
    miny = min(BTC.minC()*(1-0.1*sgn(BTC.minC())),miny);
    if (miny>0)
        miny = min(0.0,miny);

    if (maxy<0)
        maxy = max(0.0,maxy);

    plot->legend->setVisible(showlegend);
    plot->clearGraphs();
    QVector<double> x, y; // initialize with entries 0..100
    format.push_back(plotformat());
    if (format[format.size()-1].xAxisTimeFormat && ((BTC.GetT(BTC.n - 1) - BTC.GetT(0)) < 5 || BTC.GetT(BTC.n - 1)< 18264))
        format[format.size()-1].xAxisTimeFormat = false;

    if (!allowtime)
        format[format.size()-1].xAxisTimeFormat = false;

    if (format[format.size()-1].xAxisTimeFormat)
    {
#ifndef Qt6
        QDateTime start = QDateTime::fromTime_t(xtoTime(BTC.GetT(0)), QTimeZone(0));
        QDateTime end = QDateTime::fromTime_t(xtoTime(BTC.GetT(BTC.n - 1)), QTimeZone(0));
#else
        QDateTime start = QDateTime::fromSecsSinceEpoch(xtoTime(BTC.GetT(0)));
        QDateTime end = QDateTime::fromSecsSinceEpoch(xtoTime(BTC.n - 1));
#endif
        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
        dateTicker->setTickCount(10);

        //dateTicker->setTickStepStrategy( QCPAxisTickerDateTime::TickStepStrategy::tssMeetTickCount);
        QString dformat;
        if (start.secsTo(end) < 600) dformat = "mm:ss:zzz";
        if (start.secsTo(end) > 3600) dformat = "hh:mm:ss";
        if (start.daysTo(end) > 1) dformat = "MMM dd\nhh:mm:ss";
        if (start.daysTo(end) > 5) dformat = "MM.dd.yyyy\nhh:mm";
        if (start.daysTo(end) > 180) dformat = "MM.dd.yyyy\nhAP";
        if (start.daysTo(end) > 2 * 365) dformat = "MMMM\nyyyy";
        dateTicker->setDateTimeFormat(dformat);

        plot->xAxis->setTicker(dateTicker);
        plot->xAxis->setTickLabelRotation(90);
       }


    for (int i=0; i<BTC.n; ++i)
    {
      if (!format[format.size()-1].xAxisTimeFormat)
        x.push_back(BTC.GetT(i));
      else
        x.push_back(xtoTime(BTC.GetT(i)));
      y.push_back(BTC.GetC(i));
    }
    // create graph and assign data to it:
    plot->addGraph();
    plot->graph(0)->setName(QString::fromStdString(BTC.name));
    plot->graph(0)->setData(x, y);
    plot->graph(0)->setPen(QPen(colours[plot->graphCount()%10]));
    if (style=="dots")
    {
        plot->graph(format.size()-1)->setLineStyle(QCPGraph::LineStyle::lsNone);
        plot->graph(format.size()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    }
    // give the axes some labels:
    plot->xAxis->setLabel("t");
    plot->yAxis->setLabel("value");
    // set axes ranges, so we see all data:
    double tickstep=0;
    if (!format[format.size()-1].xAxisTimeFormat)
    {   plot->xAxis->setRange(BTC.GetT(0), BTC.GetT(BTC.n-1));
        tickstep = (BTC.GetT(BTC.n-1) - BTC.GetT(0))/10;
    }
    else
    {   plot->xAxis->setRange(xtoTime(BTC.GetT(0)), xtoTime(BTC.GetT(BTC.n-1)));
        tickstep = (xtoTime(BTC.GetT(BTC.n-1)) - xtoTime(BTC.GetT(0)))/10;
    }
    plot->yAxis->setRange(miny, maxy);


    plot->replot();

    return true;

}

QList<QAction *> Plotter::subActions(const QMap<QString, int> &list, const int &value, QMenu * menuItem, int graphIndex, QVariant val, bool enabled)
{
    QList <QAction *> r;
    for (int i = 0; i < list.keys().count(); i++)
    {
        QAction *a = menuItem->addAction(list.keys()[i]);
        a->setEnabled(enabled);
        r.append(a);
        r[i]->setProperty("Graph", graphIndex);
        r[i]->setProperty("Prop", val);
        r[i]->setCheckable(true);
        if (list[list.keys()[i]] == value)
            r[i]->setChecked(true);
    }
    return r;
}


void Plotter::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    if (plot->graphCount())
    {
        if (plot->graphCount() == 1)
            menu.addAction("Copy Curve");
        else
            menu.addAction("Copy Curves");

        if (parent->graphsClipboard.size() == 1)
            menu.addAction("Paste Curve");
        if (parent->graphsClipboard.size() > 1)
            menu.addAction("Paste Curves");
        menu.addSeparator();
        QMenu *prop = menu.addMenu("Graph Properties");
        QMenu *xAxis = prop->addMenu("X-Axis");
        QMenu *yAxis = prop->addMenu("Y-Axis");
        xAxis->addActions(subActions(format[0].axisTimeFormats,0,xAxis,0,"x-Axis type"));
        yAxis->addActions(subActions(format[0].axisTypes,0,yAxis,0,"y-Axis type"));
        menu.addSeparator();
        QList<QMenu *>graphs;
        for (int i = 0; i < plot->graphCount(); i++)
        {
            graphs.push_back(menu.addMenu(plot->graph(i)->name()));
            graphs[i]->setProperty("Graph", i);
            QMenu* lineStyle = graphs[i]->addMenu("Line Style");
            lineStyle->addActions(subActions(format[i].lineStyles, format[i].lineStyle, lineStyle, i, "Line Style"));

            QMenu* scatterStyle = graphs[i]->addMenu("Point Style");
            scatterStyle->addActions(subActions(format[i].scatterStyles, format[i].scatterStyle, scatterStyle, i, "Scatter Style"));

            QMenu* lineType = graphs[i]->addMenu("Line Type");
            lineType->addActions(subActions(format[i].penStyles, format[i].penStyle, lineType, i, "Line Type"));

            QMenu* width = graphs[i]->addMenu("Width");
            width->addActions(subActions(format[i].penWidths, format[i].penWidth, width, i, "Width"));

            QMenu* color = graphs[i]->addMenu("Color");
            color->addActions(subActions(format[i].colors, format[i].color, color, i, "Color"));

        }


        QAction *selectedAction = menu.exec(event->globalPos());

        if (selectedAction != nullptr)
        {
            QString graph = selectedAction->property("Graph").toString();
            QString prop = selectedAction->property("Prop").toString();
            QString text = selectedAction->text();
            if (!graph.isNull())
            {
                int i = graph.toInt();
                            previousFormat = format;
                if (prop == "xAxisType")
                    format[i].xAxisType = QCPAxis::ScaleType(format[i].axisTypes[text]);
                /*if (prop == "xAxisTimeFormat")
                {
                    bool newTimeFormat = text.toLower().contains("time");
                    if (format[i].xAxisTimeFormat != newTimeFormat)
                    {
                        format[i].xAxisTimeFormat = newTimeFormat;
                        int n = plot->graph(i)->data()->;
                        QVector<double> x(n), y(n);
                        QCPData data;
                        QList<qreal> oldX = ui->customPlot->graph(i)->data()->keys();
                        QList<QCPData> oldY = ui->customPlot->graph(i)->data()->values();
                        for (int c = 0; c < n; ++c)
                        {
                            qDebug() << c;
                            if (newTimeFormat)
                                x[c] = xtoTime(oldX[c]);
                            else
                                x[c] = timetoX(oldX[c]);
                            y[c] = oldY[c].value;
                        }
                        ui->customPlot->graph(i)->setData(x, y);

                        if (format[i].xAxisTimeFormat)
                        {
                            QDateTime start = QDateTime::fromTime_t(x[0], QTimeZone(0));
                            QDateTime end = QDateTime::fromTime_t(x[x.count() - 1], QTimeZone(0));
                            ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
                            QString format;
                            if (start.secsTo(end) < 600) format = "mm:ss:zzz";
                            if (start.secsTo(end) > 3600) format = "hh:mm:ss";
                            if (start.daysTo(end) > 1) format = "MMM dd\nhh:mm:ss";
                            if (start.daysTo(end) > 5) format = "MMM dd, yyyy\nhh:mm";
                            if (start.daysTo(end) > 180)format = "MMM dd, yyyy\nhAP";
                            if (start.daysTo(end) > 2 * 365)format = "MMMM\nyyyy";
                            ui->customPlot->xAxis->setDateTimeFormat(format);
                        }
                        else
                            ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);

                    }
                }*/

                if (prop == "yAxisType")
                    format[i].yAxisType = QCPAxis::ScaleType(format[i].axisTypes[text]);
                if (prop == "legend")
                    format[i].legend = format[i].legends[text];
                if (prop == "Line Style")
                    format[i].lineStyle = QCPGraph::LineStyle(format[i].lineStyles[text]);
                if (prop == "Scatter Style")
                    format[i].scatterStyle = QCPScatterStyle::ScatterShape(format[i].scatterStyles[text]);
                if (prop == "Line Type")
                    format[i].penStyle = Qt::PenStyle(format[i].penStyles[text]);
                if (prop == "Width")
                    format[i].penWidth = text.toInt();
                if (prop == "Color")
                    format[i].color = Qt::GlobalColor(format[i].colors[text]);
                //		if (format != previousFormat)
                refreshFormat();
            }
            if (selectedAction->text().contains("Copy"))
            {
                parent->graphsClipboard.clear();
                for (int i = 0; i < plot->graphCount(); i++)
                    parent->graphsClipboard.insert(plot->graph(i), format[i]);
                // Set the clilpboard image
                QClipboard * clipboard = QApplication::clipboard();
                QPalette mypalette = this->palette();
                mypalette.setColor(QPalette::Window, Qt::white);
                plot->setPalette(mypalette);
#ifndef Qt6
              QPixmap pixmap = QPixmap::grabWidget(this);
#else
              QPixmap pixmap = this->grab();
#endif
                clipboard->setPixmap(pixmap);
            }

            if (selectedAction->text().contains("Paste"))
                foreach (QCPGraph *g , parent->graphsClipboard.keys())
                {
                    addScatterPlot(g, parent->graphsClipboard[g]);
                }
        }

    }
}

QCPGraph* Plotter::addScatterPlot(QCPGraph *g, plotformat format)
{
    QCustomPlot *customPlot = plot;
    QPen pen;
    QCPGraph* graph = customPlot->addGraph();
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);
    pen.setColor(Qt::black);
    graph->setPen(pen); //g->pen());
    graph->setName(g->name());
    QVector<QVector<double>> timeseriesdata = QCPDatatoQVector(g);
    graph->addData(timeseriesdata[0],timeseriesdata[1]);
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->rescaleAxes();
    customPlot->replot();
    this->format.push_back(format);
    return graph;
}


CTimeSeries<timeseriesprecision> Plotter::QCPDatatoTimeSeries(QCPGraphDataContainer &_data)
{
    CTimeSeries<timeseriesprecision> out;
    for (QCPGraphDataContainer::const_iterator it=_data.begin(); it!=_data.end(); ++it)
    {
        out.append(it->key, it->value);
    }
    return out;
}

QVector<QVector<double>> Plotter::QCPDatatoQVector(QCPGraph *g)
{
    QVector<QVector<double>> out(2);
    for (QCPGraphDataContainer::const_iterator it=g->data()->begin(); it!=g->data()->end(); ++it)
    {
        out[0].append(it->key);
        out[1].append(it->value);
    }
    return out;
}


void Plotter::refreshFormat()
{
    QCustomPlot *plot1 = plot;
    plot1->xAxis->setScaleType(format[0].xAxisType);

    if (format[format.size()-1].xAxisTimeFormat)
    {
#ifndef Qt6
        QDateTime start = QDateTime::fromTime_t(minx,QTimeZone(0));
        QDateTime end = QDateTime::fromTime_t(maxx, QTimeZone(0));
#else
        QDateTime start = QDateTime::fromSecsSinceEpoch(minx);
        QDateTime end = QDateTime::fromSecsSinceEpoch(maxx);
#endif


        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
        dateTicker->setTickCount(10);
        double timeinsecs = start.secsTo(end)*86400;
        QString dformat;
        if (timeinsecs < 600) dformat = "mm:ss:zzz";
        if (timeinsecs > 3600) dformat = "hh:mm:ss";
        if (timeinsecs > 86400) dformat = "MMM dd\nhh:mm:ss";
        if (timeinsecs > 5*86400) dformat = "MM.dd.yyyy\nhh:mm";
        if (timeinsecs > 180*86400) dformat = "MM.dd.yyyy\nhAP";
        if (timeinsecs > 2 * 365*86400) dformat = "MMMM\nyyyy";
        dateTicker->setDateTimeFormat(dformat);

        plot->xAxis->setTicker(dateTicker);
        plot->xAxis->setTickLabelRotation(90);
    }

    plot1->yAxis->setScaleType(format[0].yAxisType);
    plot1->legend->setVisible(format[0].legend);
    plot1->xAxis->setLabel(format[0].xAxisLabel);
    plot1->yAxis->setLabel(format[0].yAxisLabel);

    for (unsigned int i = 0; i < format.size(); i++)
    {
        format[i].xAxisType = format[0].xAxisType;
        format[i].yAxisType = format[0].yAxisType;
        format[i].legend = format[0].legend;
        plot1->graph(i)->setLineStyle(format[i].lineStyle);
        plot1->graph(i)->setScatterStyle(format[i].scatterStyle);
        QPen pen = plot->graph(i)->pen();
        pen.setColor(format[i].color);
        pen.setWidth(format[i].penWidth);
        pen.setStyle(format[i].penStyle);
        plot1->graph(i)->setPen(pen);
    }
    plot1->rescaleAxes();
    plot1->replot();
}



bool Plotter::AddData(const CTimeSeries<timeseriesprecision>& BTC, bool allowtime, string style)
{
    maxx = max(BTC.GetT(0),maxx);
    maxy = max(BTC.maxC()*(1+0.1*sgn(BTC.maxC())),maxy);
    minx = min(BTC.GetT(BTC.n-1),minx);
    miny = min(BTC.minC()*(1-0.1*sgn(BTC.minC())),miny);

    if (miny>0)
        miny = min(0.0,miny);

    if (maxy<0)
        maxy = max(0.0,maxy);

    plot->legend->setVisible(showlegend);
    QVector<double> x, y; // initialize with entries 0..100
    format.push_back(plotformat());
    if (format[format.size()-1].xAxisTimeFormat && ((BTC.GetT(BTC.n - 1) - BTC.GetT(0)) < 5 || BTC.GetT(BTC.n - 1)< 18264))
        format[format.size()-1].xAxisTimeFormat = false;

    if (!allowtime)
        format[format.size()-1].xAxisTimeFormat = false;
    if (format[format.size()-1].xAxisTimeFormat)
    {
#ifndef Qt6
        QDateTime start = QDateTime::fromTime_t(xtoTime(BTC.GetT(0)), QTimeZone(0));
        QDateTime end = QDateTime::fromTime_t(xtoTime(BTC.GetT(BTC.n - 1)), QTimeZone(0));
#else
        QDateTime start = QDateTime::fromSecsSinceEpoch(BTC.GetT(0));
        QDateTime end = QDateTime::fromSecsSinceEpoch(BTC.GetT(BTC.n - 1));
#endif
                QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
        dateTicker->setTickCount(10);

        QString dformat;
        if (start.secsTo(end) < 600) dformat = "mm:ss:zzz";
        if (start.secsTo(end) > 3600) dformat = "hh:mm:ss";
        if (start.daysTo(end) > 1) dformat = "MMM dd\nhh:mm:ss";
        if (start.daysTo(end) > 5) dformat = "MM.dd.yyyy\nhh:mm";
        if (start.daysTo(end) > 180) dformat = "MM.dd.yyyy\nhAP";
        if (start.daysTo(end) > 2 * 365) dformat = "MMMM\nyyyy";
        dateTicker->setDateTimeFormat(dformat);

        plot->xAxis->setTicker(dateTicker);
        plot->xAxis->setTickLabelRotation(90);
       }


    for (int i=0; i<BTC.n; ++i)
    {
      if (!format[format.size()-1].xAxisTimeFormat)
        x.push_back(BTC.GetT(i));
      else
        x.push_back(xtoTime(BTC.GetT(i)));
      y.push_back(BTC.GetC(i));
    }
    // create graph and assign data to it:
    plot->addGraph();
    plot->graph(format.size()-1)->setName(QString::fromStdString(BTC.name));
    plot->graph(format.size()-1)->setData(x, y);
    plot->graph(format.size()-1)->setPen(QPen(colours[plot->graphCount()%10]));
    if (style=="dots")
    {
        plot->graph(format.size()-1)->setLineStyle(QCPGraph::LineStyle::lsNone);
        plot->graph(format.size()-1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    }
    // give the axes some labels:
    plot->xAxis->setLabel("t");
    plot->yAxis->setLabel("value");
    // set axes ranges, so we see all data:
    double tickstep=0;
    if (!format[format.size()-1].xAxisTimeFormat)
    {   plot->xAxis->setRange(BTC.GetT(0), BTC.GetT(BTC.n-1));
        tickstep = (BTC.GetT(BTC.n-1) - BTC.GetT(0))/10;
    }
    else
    {   plot->xAxis->setRange(xtoTime(BTC.GetT(0)), xtoTime(BTC.GetT(BTC.n-1)));
        tickstep = (xtoTime(BTC.GetT(BTC.n-1)) - xtoTime(BTC.GetT(0)))/10;
    }
    plot->yAxis->setRange(miny, maxy);
    return true;

}

void Plotter::SetYAxisTitle(const QString& s)
{
    plot->yAxis->setLabel(s);
}

void Plotter::SetXAxisTitle(const QString& s)
{
    plot->xAxis->setLabel(s);
}

bool Plotter::On_Legend_Clicked()
{
    showlegend = !showlegend;
    plot->legend->setVisible(showlegend);
    plot->replot();
    return true;
}

void Plotter::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
  // Set an axis label by double clicking on it
  if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
  {
    bool ok;
    QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
    if (ok)
    {
      axis->setLabel(newLabel);
      plot->replot();
    }
  }
}

void Plotter::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
  // Rename a graph by double clicking on its legend item
  Q_UNUSED(legend)
  if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
  {
    QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
    bool ok;
    QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
    if (ok)
    {
      plItem->plottable()->setName(newName);
      plot->replot();
    }
  }
}

void Plotter::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (plot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
  } else  // general context menu on graphs requested
  {
    if (plot->selectedGraphs().size() > 0)
    {  menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
       menu->addAction("Symbols", this, SLOT(turnSelectedtoSymbols()));
    }
    if (plot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
  }
  if (plot->graphCount())
  {
      if (plot->graphCount() == 1)
          menu->addAction("Copy Curve");
      else
          menu->addAction("Copy Curves");

      menu->addAction("Zoom Extends");
      if (parent->graphsClipboard.size() == 1)
          menu->addAction("Paste Curve");
      if (parent->graphsClipboard.size() > 1)
          menu->addAction("Paste Curves");
      menu->addSeparator();
      QMenu *prop = menu->addMenu("Graph Properties");
      //QMenu *xAxis = prop->addMenu("X-Axis");
      QMenu *yAxis = prop->addMenu("Y-Axis");
      //xAxis->addActions(subActions(format[0].axisTimeFormats,0,xAxis,0,"xAxisType"));

      yAxis->addActions(subActions(format[0].axisTypes,format[0].yAxisType,yAxis,0,"yAxisType"));
      menu->addSeparator();
      QList<QMenu *>graphs;
      for (int i = 0; i < plot->graphCount(); i++)
      {
          graphs.push_back(menu->addMenu(plot->graph(i)->name()));
          graphs[i]->setProperty("Graph", i);
          QMenu* lineStyle = graphs[i]->addMenu("Line Style");
          lineStyle->addActions(subActions(format[i].lineStyles, format[i].lineStyle, lineStyle, i, "Line Style"));

          QMenu* scatterStyle = graphs[i]->addMenu("Point Style");
          scatterStyle->addActions(subActions(format[i].scatterStyles, format[i].scatterStyle, scatterStyle, i, "Scatter Style"));

          QMenu* lineType = graphs[i]->addMenu("Line Type");
          lineType->addActions(subActions(format[i].penStyles, format[i].penStyle, lineType, i, "Line Type"));

          QMenu* width = graphs[i]->addMenu("Width");
          width->addActions(subActions(format[i].penWidths, format[i].penWidth, width, i, "Width"));

          QMenu* color = graphs[i]->addMenu("Color");
          color->addActions(subActions(format[i].colors, format[i].color, color, i, "Color"));

      }


      QAction *selectedAction = menu->exec(plot->mapToGlobal(pos));

      if (selectedAction != nullptr)
      {
          QString graph = selectedAction->property("Graph").toString();
          QString prop = selectedAction->property("Prop").toString();
          QString text = selectedAction->text();
          if (!graph.isNull())
          {
              int i = graph.toInt();
                          previousFormat = format;
              if (prop == "xAxisType")
              {
                  format[i].xAxisTimeFormat = format[i].axisTimeFormats[text];
              }


              if (prop == "yAxisType")
              {     format[i].yAxisType = QCPAxis::ScaleType(format[0].axisTypes[text]);
                    if (format[0].axisTypes[text] == QCPAxis::ScaleType::stLogarithmic)
                    {   QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
                        logTicker->setLogBase(10);
                        logTicker->setSubTickCount(10);
                        plot->yAxis->setTicker(logTicker);
                        plot->yAxis->setScaleType(QCPAxis::ScaleType::stLogarithmic);
                    }
                    else
                    {
                        QSharedPointer<QCPAxisTicker> linearTicker(new QCPAxisTicker);
                        linearTicker->setTickCount(10);
                        plot->yAxis->setTicker(linearTicker);
                        plot->yAxis->setScaleType(QCPAxis::ScaleType::stLinear);
                    }
              }
              if (prop == "legend")
                  format[i].legend = format[i].legends[text];
              if (prop == "Line Style")
                  format[i].lineStyle = QCPGraph::LineStyle(format[i].lineStyles[text]);
              if (prop == "Scatter Style")
                  format[i].scatterStyle = QCPScatterStyle::ScatterShape(format[i].scatterStyles[text]);
              if (prop == "Line Type")
                  format[i].penStyle = Qt::PenStyle(format[i].penStyles[text]);
              if (prop == "Width")
                  format[i].penWidth = text.toInt();
              if (prop == "Color")
                  format[i].color = Qt::GlobalColor(format[i].colors[text]);
              //		if (format != previousFormat)
              refreshFormat();
          }
          if (selectedAction->text().contains("Copy"))
          {
              parent->graphsClipboard.clear();
              for (int i = 0; i < plot->graphCount(); i++)
                  parent->graphsClipboard.insert(plot->graph(i), format[i]);
              // Set the clilpboard image
              QClipboard * clipboard = QApplication::clipboard();
              QPalette mypalette = this->palette();
              mypalette.setColor(QPalette::Window, Qt::white);
              plot->setPalette(mypalette);
#ifndef Qt6
              QPixmap pixmap = QPixmap::grabWidget(this);
#else
              QPixmap pixmap = this->grab();
#endif
              clipboard->setPixmap(pixmap);
          }
          if (selectedAction->text().contains("Zoom Extends"))
          {
              if (!format[format.size()-1].xAxisTimeFormat)
              {
                  plot->xAxis->setRange(minx, maxx);
              }
              else
              {
                  plot->xAxis->setRange(xtoTime(minx), xtoTime(maxx));
              }
              plot->yAxis->setRange(miny-0.001, maxy+0.001);
              plot->replot();

          }
          if (selectedAction->text().contains("Paste"))
              foreach (QCPGraph *g , parent->graphsClipboard.keys())
              {
                  addScatterPlot(g, parent->graphsClipboard[g]);
              }
      }

  }

}

void Plotter::moveLegend()
{
  if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      plot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      plot->replot();
    }
  }
}

void Plotter::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
  // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
  // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
  double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
  QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
  ui->statusBar->showMessage(message, 2500);
}

void Plotter::removeSelectedGraph()
{
  //qDebug()<<"Removing Selected Graph ...";
  if (plot->selectedGraphs().size() > 0)
  {
    plot->removeGraph(plot->selectedGraphs().first());
    plot->replot();
  }
}

/*void Plotter::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    plot->axisRect()->setRangeDrag(plot->xAxis->orientation());
  else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    plot->axisRect()->setRangeDrag(plot->yAxis->orientation());
  else
    plot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void Plotter::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    plot->axisRect()->setRangeZoom(plot->xAxis->orientation());
  else if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    plot->axisRect()->setRangeZoom(plot->yAxis->orientation());
  else
    plot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}*/

void Plotter::selectionChanged()
{

  if (plot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      plot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    plot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    plot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (plot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      plot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || plot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    plot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    plot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<plot->graphCount(); ++i)
  {
    QCPGraph *graph = plot->graph(i);
    QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
  }
}

void Plotter::turnSelectedtoSymbols()
{
  //qDebug()<<"Removing Selected Graph ...";
  if (plot->selectedGraphs().size() > 0)
  {
    plot->selectedGraphs().first()->setLineStyle(QCPGraph::lsNone);
    plot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    plot->replot();
  }
}

void Plotter::Deselect()
{
    for (int i=0; i<plot->graphCount(); ++i)
    {
      QCPGraph *graph = plot->graph(i);
      QCPPlottableLegendItem *item = plot->legend->itemWithPlottable(graph);
      if (item->selected() || graph->selected())
      {
        item->setSelected(false);
        //graph->setSelectable(QCP::SelectionType::aeNone);

      }
    }
}




