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
    setAcceptedMouseButtons(nullptr);

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
    

//	sourceID = source->ID;
//	destID = dest->ID;

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
    setAcceptedMouseButtons(nullptr);

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

//	sourceID = source->ID;
//	destID = dest->ID;

    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);

    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    parent->MainGraphicsScene->addItem(this);
    name = QString("%1 - %2").arg(sourceNode->Name()).arg(destNode->Name());

    //changed();
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
    this->setZValue(qrand()%100-50);

}

QVariant Edge::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        if (value==true)
            parent->mainWindow()->PopulatePropertyTable(object()->GetVars());
        break;
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
