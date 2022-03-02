#include "node.h"
#include "diagramview.h"
#include "System.h"
#include "mainwindow.h"
#include "qapplication.h"
#include <qdebug.h>
#include "qgraphicssceneevent.h"
#include "edge.h"
#include "enums.h"
#include "Object.h"
#include "qrandom.h"

Node::Node(DiagramView *_parent, System *_system)
{
    color.color1 = Qt::yellow;
    parent = _parent;
    system = _system;

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    itemType = Object_Types::Block;
    //setX(x);
    //setY(y);
    setX(1000-width/2);
    setY(1000-height/2);

    //width = max(minW, _width);
    //height = max(minH, _height);

    parent->MainGraphicsScene->addItem(this);

}

QRectF Node::boundingRect() const
{
    qreal adjust = 0;
    return QRectF( 0 - adjust, 0 - adjust, width + adjust, height + adjust);
}
QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}
void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //qDebug() << "Painting Node!"; 
    qreal iconmargin = 0;
    painter->setPen(Qt::NoPen);
    painter->setOpacity(0.7);
    QColor Color1, Color2;
    painter->setBrush(Qt::white);
    QRadialGradient radialGrad(QPointF(width / 2, height / 2), min(width, height));
    radialGrad.setColorAt(0, QColor(Qt::lightGray).light(300));
    radialGrad.setColorAt(1, QColor(Qt::lightGray).light(120));
    QPixmap pixmap;
    if (!object())
    {
        //parent->mainWindow()->RecreateGraphicItemsFromSystem();
        return;
    }
    if (QString::fromStdString(system->GetModel(object()->GetType())->IconFileName()).contains("/"))
        pixmap = QPixmap(QString::fromStdString(system->GetModel(object()->GetType())->IconFileName()));
    else
        pixmap = QPixmap(QString::fromStdString(RESOURCE_DIRECTORY+"/Icons/" + system->GetModel(object()->GetType())->IconFileName()));
    QRectF rect = QRectF(boundingRect().left()*0 + iconmargin*boundingRect().width(), boundingRect().top()*0+iconmargin*boundingRect().width(), boundingRect().width()*(1-iconmargin), boundingRect().height()*(1-iconmargin));
    QRectF source(0, 0, pixmap.size().width(), pixmap.size().height());
    painter->drawPixmap(rect, pixmap, source);

    if (isSelected())
    {
        radialGrad.setColorAt(0, Qt::green);
        radialGrad.setColorAt(1, Qt::darkGreen);
    }

    if (parent->colorCode.nodes)
        painter->setBrush(color.color1);
    else
        painter->setBrush(radialGrad);

    painter->setPen(QPen(Qt::white, (bold) ? 2 : 0));
    painter->drawRoundRect(0, 0, width, height,10,10);
    painter->setPen(QPen(Qt::black, (bold) ? 2 : 0));
    qreal factor = parent->transform().scale(1, 1).mapRect(QRectF(0, 0, 1, 1)).width();
    int size = int(4 + 6 / factor)*fontfactor();
    QFont QF = painter->font(); QF.setPointSize(size);// QF.pointSize() + 2);
    QF.setBold(bold);
    painter->setFont(QF);

    painter->drawText(10, height - 10, QString("%1: %2").arg(QString::fromStdString(object()->GetType())).arg(QString::fromStdString(object()->GetName())));
    //qDebug() << "Node Paint Complete!";
}

Object *Node::object()
{
    return system->GetObjectBasedOnPrimaryKey(objectPrimaryKey);
}

void Node::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
    bold = true;
    update();
}

void Node::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QPointF p = QPointF(x() + event->pos().x(), y() + event->pos().y());
    parent->nodeContextMenuRequested(this, p);
    this->setZValue(qrand()%100-50);

}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (Edge *edge, edgeList)
            edge->adjust();
        break;
    case QGraphicsItem::ItemSelectedChange:
        if (value==true)
            parent->mainWindow()->PopulatePropertyTable(object()->GetVars());
        break;
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

corners Node::corner(const int _x, const int _y)
{
    int border = 15;
    //qDebug()<<x()<<","<<y();
    //qDebug()<<_x<<","<<_y;
    if (abs(_x - x()) < border)
    {

        if (abs(_y - y()) < border)
            return (topleft);
        if (abs(_y - y() - height) < border)
            return (bottomleft);
    }
    if (abs(_x - x()-width) < border)
    {
        if (abs(_y - y()) < border)
            return (topright);
        if (abs(_y - y() - height) < border)
            return (bottomright);
    }
    return (corners::none);
}

void Node::addEdge(Edge *edge)
{
    edgeList << edge;
    edge->adjust();
}

edgesides Node::edge(const int _x, const int _y)
{
    int hBorder = (height > 60) ? 15 : height / 4;
    int vBorder = (width > 60) ? 15 : width / 4;
    if (corner(_x, _y)) return edgesides::noside;
    if (abs(_x - x()) < vBorder) return edgesides::leftside;
    if (abs(_y - y()) < hBorder) return edgesides::topside;
    if (abs(_y - y() - height) < hBorder) return edgesides::bottomside;
    if (abs(_x - x() - width) < vBorder) return edgesides::rightside;
    return edgesides::noside;
}

Node::Node(const Node &E)
{
    setFlags(E.flags());
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(E.zValue());
    setPos(E.pos());
    setFlag(ItemSendsGeometryChanges);
    edgeList = E.edgeList;
    newPos = E.newPos;
    name = E.name;
    width = E.Width();
    height = E.Height();
    parent = E.parent;
    itemType = E.itemType;
    objectPrimaryKey = E.objectPrimaryKey;

}

void Node::setWidth(const int& _width)
{
    width = _width; update();
}
void Node::setHeight(const int& _height) {
    height = _height; update();
}

void Node::setX(const int& x)
{
    QGraphicsItem::setX(x);
}
void Node::setY(const int& y)
{
    QGraphicsItem::setY(y);
}

void Node::SetObject(Object* _object) {
    objectPrimaryKey = _object->GetPrimaryKey();
    setX(_object->GetProperty("x"));
    setY(_object->GetProperty("y"));
    setWidth(max(_object->GetProperty("_width"),double(minW)));
    setHeight(max(_object->GetProperty("_height"),double(minH)));
    //qDebug() << "Node Position: " << x() << "," << y() << "," << width << "," << height; 
    //qDebug() << "Node Position: " << _object->GetProperty("x") << "," << _object->GetProperty("y") << "," << width << "," << height;
}

QString Node::Name() { 
    if (!object())
    {
        return "";
    }
    return QString::fromStdString(object()->GetName());

}

double Node::fontfactor(){
    if (parent)
        return parent->fontfactor;
    else
        return 1;
}
