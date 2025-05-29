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


#pragma once
 
#include <QPoint>
#include "qcustomplot.h"
 
class QRubberBand;
class QMouseEvent;
class QWidget;
 
class CustomPlotZoom : public QCustomPlot
{
   
 
public:
    CustomPlotZoom(QWidget * parent = 0);
    virtual ~CustomPlotZoom();
 
    void setZoomMode(bool mode);
 
private slots:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
	void wheelEvent(QWheelEvent * event);
    void mouseReleaseEvent(QMouseEvent * event) override;
	void axisDoubleClick(QCPAxis *  axis, QCPAxis::SelectablePart  part, QMouseEvent *  event);
private:
    bool mZoomMode;
    QRubberBand * mRubberBand;
    QPoint mOrigin;
};