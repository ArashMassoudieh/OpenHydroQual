#include "runtimewindow.h"
#include "ui_runtimewindow.h"

RunTimeWindow::RunTimeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunTimeWindow)
{
    ui->setupUi(this);
    plot = new QCustomPlot(this);
    plot->setObjectName(QStringLiteral("RunProgressPlot"));
    ui->horizontalLayout->addWidget(plot);
    QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(2);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(plot->sizePolicy().hasHeightForWidth());
    plot->setSizePolicy(sizePolicy2);

    plot->addGraph();
    plot->graph(0)->setName("Time step size");

    plot->graph(0)->setPen(QPen(Qt::blue));

    plot->xAxis->setLabel("t");
    plot->yAxis->setLabel("Time-step size");
    // set axes ranges, so we see all data:

    plot->replot();
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);

}

RunTimeWindow::~RunTimeWindow()
{
    delete ui;
}

void RunTimeWindow::AppendText(const QString &s)
{
    ui->textBrowser->append(s);
}

void RunTimeWindow::AppendErrorMessage(const QString &s)
{
    ui->textBrowser->setTextColor(Qt::red);
    ui->textBrowser->append(s);
    ui->textBrowser->setTextColor(Qt::black);
}

void RunTimeWindow::AddDataPoint(const double &t, const double value)
{
    plot->graph(0)->addData(t,value);
}
void RunTimeWindow::SetProgress(const double &val)
{
    ui->progressBar->setValue(int(val*100));
}

void RunTimeWindow::SetXRange(const double &tstart, const double &tend)
{
    plot->xAxis->setRange(tstart,tend);
}

void RunTimeWindow::SetYRange(const double &ymin, const double &ymax)
{
     plot->yAxis->setRange(ymin,ymax);
}

void RunTimeWindow::SetUpForForwardRun()
{
    ui->optprogressBar->setVisible(false);
}
