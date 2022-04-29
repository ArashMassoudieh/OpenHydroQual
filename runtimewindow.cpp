#include "runtimewindow.h"
#include "ui_runtimewindow.h"
#include "mainwindow.h"

RunTimeWindow::RunTimeWindow(QWidget *parent, config cfg) :
    QDialog(parent),
    ui(new Ui::RunTimeWindow)
{
    ui->setupUi(this);

    QIcon mainicon(QString::fromStdString(RESOURCE_DIRECTORY)+"/Icons/Aquifolium.png");
    setWindowIcon(mainicon);

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->textBrowserdetails->setVisible(false);
    connect(ui->ShowDetails,SIGNAL(clicked()),this,SLOT(showdetails()));
    connect(ui->pushButtonStop,SIGNAL(clicked()),this,SLOT(stop_triggered()));

    SetUp(cfg);
}

RunTimeWindow::~RunTimeWindow()
{
    delete ui;
}

void RunTimeWindow::AppendText(const QString &s)
{
    ui->textBrowser->append(s);
}

void RunTimeWindow::AppendText(const std::string &s)
{
    ui->textBrowser->append(QString::fromStdString(s));
}

void RunTimeWindow::AppendtoDetails(const QString &s)
{
    ui->textBrowserdetails->append(s);
}


void RunTimeWindow::AppendErrorMessage(const QString &s)
{
#pragma omp critical
    {   ui->textBrowser->setTextColor(Qt::red);
        ui->textBrowser->append(s);
        ui->textBrowser->setTextColor(Qt::black);
    }
}

void RunTimeWindow::AddDataPoint(const double &t, const double value, int graph_no)
{
    if (graph_no==0)
    {   plot->graph(0)->addData(t,value);
        if (value>plot->yAxis->range().upper)
            plot->yAxis->setRange(plot->yAxis->range().lower,value*1.3);
        if (value<plot->yAxis->range().lower)
            plot->yAxis->setRange(value*1.3,plot->yAxis->range().upper);
    }
    if (graph_no==1)
        if (plot2)
        {   plot2->graph(0)->addData(t,value);
            if (value>plot2->yAxis->range().upper)
                plot2->yAxis->setRange(plot->yAxis->range().lower,value*1.3);
            if (value<plot->yAxis->range().lower)
                plot2->yAxis->setRange(value*1.3,plot->yAxis->range().upper);
        }


}

void RunTimeWindow::Replot()
{
    plot->replot();
    if (plot2)
        plot2->replot();
}
void RunTimeWindow::SetProgress(const double &val)
{
    ui->progressBar->setValue(int(val*100));
}

void RunTimeWindow::SetProgress2(const double &val)
{
    ui->optprogressBar->setValue(int(val*100));
}


void RunTimeWindow::SetXRange(const double &tstart, const double &tend, int plotno)
{
    if (plotno==0)
        plot->xAxis->setRange(tstart,tend);
    else if (plotno==1)
        if (plot2)
            plot2->xAxis->setRange(tstart,tend);
}

void RunTimeWindow::SetYRange(const double &ymin, const double &ymax)
{
     plot->yAxis->setRange(ymin,ymax);
}

void RunTimeWindow::SetUp(config cnfg)
{

    plot = new QCustomPlot(this);
    plot->setObjectName(QStringLiteral("RunProgressPlot"));

    ui->verticalLayout_graphs->insertWidget(0,plot);
    QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(2);
    sizePolicy2.setVerticalStretch(3);
    sizePolicy2.setHeightForWidth(plot->sizePolicy().hasHeightForWidth());
    plot->setSizePolicy(sizePolicy2);

    plot->addGraph();
    plot->graph(0)->setName("Time step size");

    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));

    // set axes ranges, so we see all data:

    plot->replot();

    if (cnfg==config::forward)
    {   ui->optprogressBar->setVisible(false);
        plot->xAxis->setLabel("t");
        plot->yAxis->setLabel("Time-step size");
        ui->optprogressBar->setVisible(false);
        return;
    }
    if (cnfg==config::optimize)
    {
        plot->graph(0)->setName("Objective function");
        plot->xAxis->setLabel("Generation");
        plot->yAxis->setLabel("Objective function");
        return;
    }
    if (cnfg == config::inverse)
    {
        plot->graph(0)->setName("Fitness");
        plot->xAxis->setLabel("Generation");
        plot->yAxis->setLabel("Fitness");
        return;
    }
    if (cnfg == config::mcmc)
    {

        plot->graph(0)->setName("Acceptance rate");
        plot->xAxis->setLabel("Sample");
        plot->yAxis->setLabel("Acceptance rate");
        plot->yAxis->setRange(0,1);

        plot2 = new QCustomPlot(this);
        plot2->setObjectName(QStringLiteral("RunProgressPlot"));
        plot2->addGraph();
        plot2->graph(0)->setName("Purtubation factor");
        plot2->xAxis->setLabel("Sample");
        plot2->yAxis->setLabel("Purturbation factor");
        plot2->yAxis->setRange(0,2);
        ui->verticalLayout_graphs->insertWidget(1,plot2);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(2);
        sizePolicy2.setVerticalStretch(3);
        sizePolicy2.setHeightForWidth(plot2->sizePolicy().hasHeightForWidth());
        plot2->setSizePolicy(sizePolicy2);

        plot2->addGraph();
        plot2->graph(0)->setName("Sample");

        plot2->graph(0)->setPen(QPen(Qt::blue));
        plot2->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
        ui->optprogressBar->setVisible(false);
        return;
    }


}

void RunTimeWindow::showdetails()
{
    if (!ui->textBrowserdetails->isVisible())
    {
        ui->ShowDetails->setText("Hide details");
        ui->textBrowserdetails->setVisible(true);
        detailson = true;
    }
    else
    {
        ui->ShowDetails->setText("Shows details");
        ui->textBrowserdetails->setVisible(false);
        detailson = false;
    }
}

void RunTimeWindow::stop_triggered() {
    stoptriggered = true;
    //qDebug() << "Stop Triggered!";
}
