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


#include "svgviewer.h"

SVGViewer::SVGViewer(QWidget *parent)
    : QGraphicsView(parent)
{
}

void SVGViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Emit the doubleClicked signal with the scene position of the click
    emit doubleClicked(mapToScene(event->pos()));

    // Call the base class implementation (optional)
    QGraphicsView::mouseDoubleClickEvent(event);

}
