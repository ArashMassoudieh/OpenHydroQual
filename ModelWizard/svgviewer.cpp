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