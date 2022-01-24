#ifndef PLOTTER_H
#define PLOTTER_H

class MainWindow;

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <BTCSet.h>
#include "qcustomplot.h"
#include "CustomPlotZoom.h"
#include "Quan.h"
#include "System.h"

struct plotformat{
    Qt::GlobalColor color = Qt::red;
    QBrush brush = QBrush(QColor(240, 255, 200));
    QCPGraph *fillGraph = 0;
    QCPGraph::LineStyle lineStyle = QCPGraph::lsLine;
    QCPScatterStyle::ScatterShape scatterStyle = QCPScatterStyle::ssNone;// ssDisc;
    bool legend = true;
    Qt::PenStyle penStyle = Qt::DotLine;
    int penWidth = 2;
    QCPAxis::ScaleType xAxisType = QCPAxis::stLinear;
    QCPAxis::ScaleType yAxisType = QCPAxis::stLinear;
    QString xAxisLabel = "", yAxisLabel = "";
    bool xAxisTimeFormat = true;
    QMap<QString, int> lineStyles = QMap<QString, int>{ { "Line", QCPGraph::lsLine }, { "None", QCPGraph::lsNone } };
    QMap<QString, int> scatterStyles = QMap<QString, int>{ { "None", QCPScatterStyle::ssNone }, { "Cross", QCPScatterStyle::ssCross }, { "Cross Circle", QCPScatterStyle::ssCrossCircle }, { "Dot", QCPScatterStyle::ssDot } };
    QMap<QString, int> axisTypes = QMap<QString, int>{ { "Normal", QCPAxis::ScaleType::stLinear }, { "Log", QCPAxis::ScaleType::stLogarithmic } };
    QMap<QString, int> axisTimeFormats = QMap<QString, int>{ { "Number", 0 },{ "Time", 1 } };
    QMap<QString, int> penStyles = QMap<QString, int>{ { "Solid Line", Qt::SolidLine }, { "Dot Line", Qt::DotLine }, { "Dash Line", Qt::DashLine }, { "Dash Dot Line", Qt::DashDotLine }, { "Dash Dot Dot Line", Qt::DashDotDotLine } };
    QMap<QString, int> colors = QMap<QString, int> { { "Red", Qt::red }, { "Black", Qt::black }, { "Blue", Qt::blue }, { "Green", Qt::green }, { "Cyan", Qt::cyan }, { "Dark Green", Qt::darkGreen },
    { "Dark Red", Qt::darkRed }, { "Dark Blue", Qt::darkBlue }, { "Dark Gray", Qt::darkGray }, { "Magenta", Qt::magenta }, { "Light Gray", Qt::lightGray }, { "Yellow", Qt::yellow } };
    QMap<QString, int> penWidths = QMap<QString, int>{ { "1", 1 }, { "2", 2 }, { "3", 3 }, { "4", 4 }, { "5", 5 } };
    QMap<QString, int> legends = QMap<QString, int>{ { "Show", 1 }, { "Hide", 0 } };

};


struct _timeseries
{
    QString filename;
    QString name;
    CTimeSeriesSet<outputtimeseriesprecision> Data;
};

namespace Ui {
class Plotter;
}


class Plotter : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plotter(MainWindow *parent = nullptr);
    ~Plotter();
    bool PlotData(const CTimeSeries<outputtimeseriesprecision>& BTC, bool allowtime=true, string style="line");
    bool PlotData(const CTimeSeriesSet<timeseriesprecision>& BTC, bool allowtime=true, string style="line");
    bool AddData(const CTimeSeries<outputtimeseriesprecision>& BTC,bool allowtime=true, string style="line");
    void SetYAxisTitle(const QString& s);
    void SetXAxisTitle(const QString& s);
    void SetTimeFormat(bool timedate);
    void SetLegend(bool onoff)
    {
        plot->legend->setVisible(onoff);
    }
private:
    Ui::Plotter *ui;
    QMap<QString, _timeseries> BTCs;
    CustomPlotZoom *plot;
    outputtimeseriesprecision minx=1e12;
    outputtimeseriesprecision maxx=-1e12;
    outputtimeseriesprecision miny=1e12;
    outputtimeseriesprecision maxy=-1e12;
    double xtoTime(const double &x) {
        return x * 86400 - 2209161600;
    }
    double timetoX(const double &time) {
        return (time + 2209161600) / 86400;
    }

    QVector<QColor> colours = {QColor("cyan"), QColor("magenta"), QColor("red"),
                          QColor("darkRed"), QColor("darkCyan"), QColor("darkMagenta"),
                          QColor("green"), QColor("darkGreen"), QColor("yellow"),
                          QColor("blue")};
    bool showlegend = true;
    MainWindow *parent = nullptr;
    vector<plotformat> format, previousFormat;
    QList<QAction *> subActions(const QMap<QString, int> &list, const int &value, QMenu * menuItem, int graphIndex, QVariant val, bool enabled = true);
    void refreshFormat();
    QCPGraph* addScatterPlot(QCPGraph *g, plotformat format);
    CTimeSeries<outputtimeseriesprecision> QCPDatatoTimeSeries(QCPGraphDataContainer &_data);
    QVector<QVector<double>> QCPDatatoQVector(QCPGraph *_g);


private slots:
    bool On_Legend_Clicked();
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void contextMenuRequest(QPoint pos);
    void moveLegend();
    void graphClicked(QCPAbstractPlottable *plottable, int dataIndex);
    void removeSelectedGraph();
    //void mousePress();
    //void mouseWheel();
    void selectionChanged();
    void turnSelectedtoSymbols();
    void Deselect();
    void contextMenuEvent(QContextMenuEvent *event);
};



#endif // PLOTTER_H
