#include "plotter.h"
#include "ui_plotter.h"
#include "qcustomplot.h"
#include "qdebug.h"
#include "qfileinfo.h"
#include "qaction.h"



Plotter::Plotter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Plotter)
{
    ui->setupUi(this);
    plot = new QCustomPlot(ui->centralWidget);
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



bool Plotter::PlotData(CBTC& BTC)
{
    minx=1e12;
    maxx=-1e12;
    miny=1e12;
    maxy=-1e12;
    maxx = max(BTC.t[0],maxx);
    maxy = max(BTC.maxC(),maxy);
    minx = min(BTC.t[BTC.n-1],minx);
    miny = min(BTC.minC(),miny);
    plot->legend->setVisible(showlegend);
    plot->clearGraphs();
    QVector<double> x, y; // initialize with entries 0..100
    for (int i=0; i<BTC.n; ++i)
    {
      x.push_back(BTC.t[i]);
      y.push_back(BTC.C[i]);
    }
    // create graph and assign data to it:
    plot->addGraph();
    plot->graph(0)->setName(QString::fromStdString(BTC.name));
    plot->graph(0)->setData(x, y);
    plot->graph(0)->setPen(QPen(colours[plot->graphCount()%10]));
    // give the axes some labels:
    plot->xAxis->setLabel("t");
    plot->yAxis->setLabel("value");
    // set axes ranges, so we see all data:
    plot->xAxis->setRange(BTC.t[0], BTC.t[BTC.n-1]);
    plot->yAxis->setRange(BTC.minC()-0.001, BTC.maxC()+0.001);
    plot->replot();

    return true;

}

bool Plotter::AddData(CBTC& BTC)
{
    plot->legend->setVisible(showlegend);
    maxx = max(BTC.t[0],maxx);
    maxy = max(BTC.maxC(),maxy);
    minx = min(BTC.t[BTC.n-1],minx);
    miny = min(BTC.minC(),miny);
    qDebug() << maxx << "," << minx << "," << miny << "," << maxy;
    QVector<double> x, y; // initialize with entries 0..100
    for (int i=0; i<BTC.n; ++i)
    {
      x.push_back(BTC.t[i]);
      y.push_back(BTC.C[i]);
    }
    // create graph and assign data to it:
    plot->addGraph();
    plot->graph(plot->graphCount()-1)->setName(QString::fromStdString(BTC.name));
    plot->graph(plot->graphCount()-1)->setData(x, y);
    plot->graph(plot->graphCount()-1)->setPen(QPen(colours[plot->graphCount()%10]));
    // give the axes some labels:
    plot->xAxis->setLabel("t");
    plot->yAxis->setLabel("value");
    // set axes ranges, so we see all data:
    plot->xAxis->setRange(minx, maxx);
    plot->yAxis->setRange(miny, maxy);
    plot->replot();

    return true;

}

void Plotter::SetYAxisTitle(const QString& s)
{
    plot->yAxis->setLabel(s);
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

  menu->popup(plot->mapToGlobal(pos));
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
  qDebug()<<"Removing Selected Graph ...";
  if (plot->selectedGraphs().size() > 0)
  {
    plot->removeGraph(plot->selectedGraphs().first());
    plot->replot();
  }
}

void Plotter::mousePress()
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
}

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
  qDebug()<<"Removing Selected Graph ...";
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


