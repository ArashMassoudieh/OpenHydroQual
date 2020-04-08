#include "edge.h"
#include "node.h"
#include "qpainter.h"
#include "diagramview.h"
#include "mainwindow.h"

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;


Edge::Edge(DiagramView *_parent)
{
    parent = _parent;
}

void Edge::adjust()
{
    if (!source || !dest)
        return;

    QLineF line(mapFromItem(source, source->Width() / 2, source->Height() / 2), mapFromItem(dest, dest->Width() / 2, dest->Height() / 2));
    qreal length = line.length();

    prepareGeometryChange();

    qreal Ox, Oy, Dx, Dy;

    if (abs(line.dx()) < 1)
    {
        Ox = 0;
        Dx = 0;
    }
    else
    {
        Ox = line.dx() / abs(line.dx())*min(int(source->Width() / 2), int(fabs(source->Height() / 2.0 * line.dx() / (line.dy()+0.5))));
        Dx = -line.dx() / abs(line.dx())*min(int(dest->Width() / 2), int(fabs(dest->Height() / 2.0 * line.dx() / (line.dy()+0.5))));
    }
    if (abs(line.dy()) < 1)
    {
        Oy = 0;
        Dy = 0;
    }
    else
    {
        Oy = line.dy() / abs(line.dy())*min(int(source->Height() / 2), int(fabs(source->Width()/ 2.0 * line.dy() / (line.dx()+0.5))));
        Dy = -line.dy() / abs(line.dy())*min(int(dest->Height() / 2), int(fabs(dest->Width() / 2.0 * line.dy() / (line.dx()+0.5))));
    }
        QPointF edgeOffsetSource(Ox, Oy);
        QPointF edgeOffsetDest(Dx, Dy);
        sourcePoint = line.p1() + edgeOffsetSource;
        destPoint = line.p2()  + edgeOffsetDest;
}

QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 8.0;

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;

    // Draw the line itself
    if (isSelected())
        painter->setPen(QPen(Qt::green, (bold) ? 3 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else if (errorDetected())
        painter->setPen(QPen(Qt::red, (bold) ? 3 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else if (parent->colorCode.edges)
        painter->setPen(QPen(color.color1, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //		painter->setPen(QPen(QColor::fromRgb(color.color1), (bold) ? 3 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
        painter->setPen(QPen(Qt::black, (bold) ? 3 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (!avoidCrossObjects)
    {

        painter->drawLine(line);

        // Draw the arrows
        double angle = ::acos(line.dx() / line.length());
        if (line.dy() >= 0)
            angle = TwoPi - angle;
        QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
            cos(angle - Pi / 3) * arrowSize);
        QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
            cos(angle - Pi + Pi / 3) * arrowSize);
        if (isSelected())
            painter->setBrush(Qt::green);
        else if (parent->colorCode.edges)
            painter->setBrush(color.color1);
        else
            painter->setBrush(Qt::black);
        painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }
    if (avoidCrossObjects)
    {

        painter->drawLine(line);

        // Draw the arrows
        double angle = ::acos(line.dx() / line.length());
        if (line.dy() >= 0)
            angle = TwoPi - angle;

        QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
            cos(angle - Pi / 3) * arrowSize);
        QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
            cos(angle - Pi + Pi / 3) * arrowSize);
        if (isSelected())
            painter->setBrush(Qt::green);
        else
            painter->setBrush(Qt::black);
        painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }
    //if (isSelected() && parent->mainWindow()->Table) Need to fix
    //{
    //    parent->tableProp->setModel(model);
    //    parent->tableProp->setFocus();
    //}
}

Object *Edge::object()
{
    return system->GetObjectBasedOnPrimaryKey(objectPrimaryKey);
}

void Edge::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    bold = true;
    update();
}
