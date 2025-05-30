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


#include "edge.h"
#include "node.h"
#include "qpainter.h"
#include "diagramview.h"
#include "mainwindow.h"
#include "QGraphicsSceneContextMenuEvent"

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;


Edge::Edge(DiagramView *_parent)
{
    parent = _parent;
}

Edge::Edge(Node *sourceNode, Node *destNode, const QString &edgeType, DiagramView *_parent)
{
    parent = _parent;
    system = _parent->mainWindow()->GetSystem();
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    source = sourceNode;
    dest = destNode;

    QList<Node*> list;
    foreach (Edge *e , source->edges())
    {
        if (e->sourceNode() == source) list.append(e->destNode());
        if (e->destNode() == source) list.append(e->sourceNode());
    }
    if (list.contains(dest))
    {
        delete this;
        return;
    }

    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    parent->MainGraphicsScene->addItem(this);
    name = QString("%1 - %2").arg(sourceNode->Name()).arg(destNode->Name());
    if (!parent->mainWindow()->AddLink(name, sourceNode->Name(), destNode->Name(), edgeType, this))
    {
        parent->MainGraphicsScene->removeItem(this);
        return; 
    }
    
    source->addEdge(this);
    dest->addEdge(this);
    adjust();
    //changed();
}

Edge::Edge(Node *sourceNode, Node *destNode, DiagramView *_parent)
{
    parent = _parent;
    system = _parent->mainWindow()->GetSystem();
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

    source = sourceNode;
    dest = destNode;

    QList<Node*> list;
    foreach (Edge *e , source->edges())
    {
        if (e->sourceNode() == source) list.append(e->destNode());
        if (e->destNode() == source) list.append(e->sourceNode());
    }
    if (list.contains(dest))
    {
        //_parent->log(QString("Duplicate connector from %1 to %2.").arg(source->Name()).arg(dest->Name()));
        delete this;
        return;
    }
    source->addEdge(this);
    dest->addEdge(this);
    adjust();

    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    parent->MainGraphicsScene->addItem(this);
    name = QString("%1 - %2").arg(sourceNode->Name()).arg(destNode->Name());

}


Edge::Edge(Edge &E)
{
    source = E.source;
    dest = E.dest;
    sourcePoint = E.sourcePoint;
    destPoint = E.destPoint;
    arrowSize = E.arrowSize;
    parent = E.parent;
    avoidCrossObjects = E.avoidCrossObjects;
    itemType = E.itemType;
}


void Edge::adjust()
{
    if (!source || !dest)
        return;

    QLineF line(mapFromItem(source, source->Width() / 2, source->Height() / 2), mapFromItem(dest, dest->Width() / 2, dest->Height() / 2));

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
    if (!object())
        return; 
    QColor objectcolor;
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;


    // Draw the line itself
    if (isSelected())
        painter->setPen(QPen(Qt::green, (bold) ? 3*parent->linkthickness : 1*parent->linkthickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else if (errorDetected())
        painter->setPen(QPen(Qt::red, (bold) ? 3*parent->linkthickness : 1*parent->linkthickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else if (parent->colorCode.edges)
        painter->setPen(QPen(color.color1, 4*parent->linkthickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //		painter->setPen(QPen(QColor::fromRgb(color.color1), (bold) ? 3 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
    {
        if (object()->HasQuantity("color"))
            objectcolor = GetColor(object()->Variable("color")->GetProperty());
        else
            objectcolor = Qt::black;
        painter->setPen(QPen(objectcolor, (bold) ? 3*parent->linkthickness : 1*parent->linkthickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }
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
            painter->setBrush(objectcolor);
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
            painter->setBrush(objectcolor);
        painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }
    if (parent->showlinkicons)
    {   qreal iconmargin = 0;
        QPixmap pixmap;

        if (QString::fromStdString(system->GetModel(object()->GetType())->IconFileName()).contains("/"))
            pixmap = QPixmap(QString::fromStdString(system->GetModel(object()->GetType())->IconFileName()));
        else
            pixmap = QPixmap(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/" + system->GetModel(object()->GetType())->IconFileName()));
        QRectF rect = QRectF(0.6*boundingRect().left()+0.4*boundingRect().right() + iconmargin*boundingRect().width(), boundingRect().top()*0.6+boundingRect().bottom()*0.4+iconmargin*boundingRect().width(), min(max(boundingRect().width()*(1-iconmargin)*0.2, boundingRect().height()*(1-iconmargin)*0.2),50.0),min(max(boundingRect().width()*(1-iconmargin)*0.2, boundingRect().height()*(1-iconmargin)*0.2),50.0));
        QRectF source(0, 0, pixmap.size().width(), pixmap.size().height());
        painter->drawPixmap(rect, pixmap, source);
    }
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

int Edge::dist(const QPointF point)
{
    int x1 = int(sourcePoint.x());
    int y1 = int(sourcePoint.y());
    int x2 = int(destPoint.x());
    int y2 = int(destPoint.y());
    int x0 = int(point.x());
    int y0 = int(point.y());
    int dist = int(abs(x0*(y2 - y1) - y0*(x2 - x1) + x2*y1 - y2*x1) / sqrt((y2 - y1) ^ 2 + (x2 - x1) ^ 2));
    return dist;
}

void Edge::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QPointF p = QPointF(x() + event->pos().x(), y() + event->pos().y());
    parent->edgeContextMenuRequested(this, p);
    this->setZValue(rand()%100-50);

}

QVariant Edge::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        if (value==true)
        {   parent->mainWindow()->PopulatePropertyTable(object()->GetVars());
            parent->mainWindow()->SetPropertyWindowTitle(QString::fromStdString(object()->GetName())+":"+QString::fromStdString(object()->GetVars()->Description()));
            QString iconfilename;
            if (object()->GetVars()->IconFileName()!="")
            {
                if (QString::fromStdString(object()->GetVars()->IconFileName()).contains("/"))
                {
                    if (QFile::exists(QString::fromStdString(object()->GetVars()->IconFileName())))
                        iconfilename = QString::fromStdString(object()->GetVars()->IconFileName());
                }
                else
                {
                    if (QFile::exists(QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + object()->GetVars()->IconFileName())))
                        iconfilename = QString::fromStdString(RESOURCE_DIRECTORY + "/Icons/" + object()->GetVars()->IconFileName());
                }

                parent->mainWindow()->SetPropertyWindowIcon(iconfilename);
            }
        }
        else
        {   parent->mainWindow()->SetPropertyWindowTitle("");
            parent->mainWindow()->SetPropertyWindowIcon("");
        }
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

QColor Edge::GetColor(const string &clrstring)
{
    vector<double> clrrgb=aquiutils::ATOF(aquiutils::split(clrstring,','));
    QColor clr;
    if (clrrgb.size()==3)
        clr = QColor(clrrgb[0],clrrgb[1],clrrgb[2]);
    else if (clrrgb.size()==2)
        clr = QColor(clrrgb[0],clrrgb[1],0);
    else if (clrrgb.size()==1)
        clr = QColor(clrrgb[0],0,0);
    else
        clr = QColor(0,0,0);

    return clr;
}

QPainterPath Edge::shape() const {
    auto path = QPainterPath{};

    auto line = QLineF{sourcePoint, destPoint};

    if (qFuzzyCompare(line.length(), qreal(0.))) {
        path.addRect(boundingRect());
        return path;
    }

    auto polygon = QPolygonF{};

    if (std::abs(line.dx()) > std::abs(line.dy())) {
        auto half_height = parent->linkthickness + arrowSize;

        // Points 1 and 2
        polygon.append({sourcePoint.x(), sourcePoint.y() - half_height});
        polygon.append({sourcePoint.x(), sourcePoint.y() + half_height});

        // Points 3 and 4
        polygon.append({destPoint.x(), destPoint.y() + half_height});
        polygon.append({destPoint.x(), destPoint.y() - half_height});

        // Point 5 = point 1
        polygon.append({sourcePoint.x(), sourcePoint.y() - half_height});
    } else {
        auto half_width = parent->linkthickness + arrowSize;

        // Points 1 and 2
        polygon.append({sourcePoint.x() - half_width, sourcePoint.y()});
        polygon.append({sourcePoint.x() + half_width, sourcePoint.y()});

        // Points 3 and 4
        polygon.append({destPoint.x() + half_width, destPoint.y()});
        polygon.append({destPoint.x() - half_width, destPoint.y()});

        // Point 5 = point 1
        polygon.append({sourcePoint.x() - half_width, sourcePoint.y()});
    }

    path.addPolygon(polygon);
    return path;
}


