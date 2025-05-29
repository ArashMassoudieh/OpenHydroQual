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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#ifndef SVGViewer_H
#define SVGViewer_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>

class SVGViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SVGViewer(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent* event) override;
signals:
    // Custom signal emitted on double-click
    void doubleClicked(const QPointF &scenePos);

protected:
    // Override the mouseDoubleClickEvent
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // SVGViewer_H
