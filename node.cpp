#include "node.h"
#include "diagramview.h"
#include "System.h"



Node::Node(DiagramView *_parent, System *_system)
{
    parent = _parent;
    system = _system;

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    itemType = Object_Types::BlockType;
    //setX(x);
    //setY(y);

    //width = max(minW, _width);
    //height = max(minH, _height);

    parent->MainGraphicsScene->addItem(this);

}

QRectF Node::boundingRect() const
{
    qreal adjust = 20;
    return QRectF( 0 - adjust, 0 - adjust, width + adjust, height + adjust);
}
QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}
void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setOpacity(0.7);
    QColor Color1, Color2;
    setX(1000);
    setY(1000);
    painter->setBrush(Qt::darkGray);
    QRadialGradient radialGrad(QPointF(width / 2, height / 2), min(width, height));
    radialGrad.setColorAt(0, QColor(Qt::lightGray).light(300));
    radialGrad.setColorAt(1, QColor(Qt::lightGray).light(120));
    QPixmap pixmap(iconfilename);
    QRectF rect = QRectF(boundingRect().left()*0 + 0.05*boundingRect().width(), boundingRect().top()*0+0.05*boundingRect().width(), boundingRect().width()*0.9, boundingRect().height()*0.9);
    QRectF source(0.0, 0.0, 512, 512);
    painter->drawPixmap(rect, pixmap, source);

    if (isSelected())
    {
        radialGrad.setColorAt(0, Qt::green);
        radialGrad.setColorAt(1, Qt::darkGreen);
    }


    painter->setPen(QPen(Qt::black, (bold) ? 2 : 0));
    painter->drawEllipse(0, 0, width, height);
    qreal factor = parent->transform().scale(1, 1).mapRect(QRectF(0, 0, 1, 1)).width();
    int size = int(4 + 6 / factor);
    QFont QF = painter->font(); QF.setPointSize(size);// QF.pointSize() + 2);
    QF.setBold(bold);
    painter->setFont(QF);

    painter->drawText(10, height - 10, QString("%1: %2").arg(QString::fromStdString(object->GetType())).arg(QString::fromStdString(object->GetName())));

}

