#ifndef PLOTTER_H
#define PLOTTER_H

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <BTCSet.h>
#include "qcustomplot.h"

struct _timeseries
{
    QString filename;
    QString name;
    CTimeSeriesSet Data;
};

namespace Ui {
class Plotter;
}

class Plotter : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plotter(QWidget *parent = nullptr);
    ~Plotter();
    bool PlotData(CBTC& BTC);
    bool AddData(CBTC& BTC);
    void SetYAxisTitle(const QString& s);
private:
    Ui::Plotter *ui;
    QMap<QString, _timeseries> BTCs;
    QCustomPlot *plot;
    double minx=1e12;
    double maxx=-1e12;
    double miny=1e12;
    double maxy=-1e12;

    QVector<QColor> colours = {QColor("cyan"), QColor("magenta"), QColor("red"),
                          QColor("darkRed"), QColor("darkCyan"), QColor("darkMagenta"),
                          QColor("green"), QColor("darkGreen"), QColor("yellow"),
                          QColor("blue")};
    bool showlegend = true;


private slots:
    bool On_Legend_Clicked();
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void removeSelectedGraph();
    void mousePress();
    void mouseWheel();
    void selectionChanged();
    void turnSelectedtoSymbols();
    void Deselect();

};



#endif // PLOTTER_H
