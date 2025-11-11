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

#include "VisualizationGraphicsView.h"
#include <QPainter>

VisualizationGraphicsView::VisualizationGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    // Enable anti-aliasing for smooth graphics
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);

    // Enable click and drag panning
    setDragMode(QGraphicsView::ScrollHandDrag);

    // Set transformation anchor to mouse position for intuitive zooming
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // Enable scroll bars
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Set background color
    setBackgroundBrush(QBrush(QColor(245, 245, 245)));
}

void VisualizationGraphicsView::wheelEvent(QWheelEvent* event)
{
    // Zoom with mouse wheel
    if (event->angleDelta().y() > 0)
    {
        // Zoom in
        scale(ZOOM_FACTOR, ZOOM_FACTOR);
    }
    else
    {
        // Zoom out
        scale(1.0 / ZOOM_FACTOR, 1.0 / ZOOM_FACTOR);
    }
}
