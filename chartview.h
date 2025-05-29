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


#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>
#include <QtWidgets/QRubberBand>

QT_USE_NAMESPACE


class MainWindow;
class QPlotWindow;

class ChartView : public QChartView

{
public:
    ChartView(QChart *chart, QPlotWindow *plotWindow, MainWindow *parent);
    QPlotWindow* PlotWindow() {return plotWindow;}
    void SetYLogAxis(bool val);
    bool Ylog() {return logYAxis;}

protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent( QMouseEvent * e );

private:
    bool m_isTouching;
    QPointF m_lastMousePos;
    bool double_clicked = false;
    MainWindow *parent;
    QPlotWindow *plotWindow;
    bool logYAxis = false;
};

#endif
