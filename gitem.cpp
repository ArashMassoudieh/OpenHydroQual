#include "gitem.h"

GItem::GItem()
{
    Pressed = false;
    setFlag(ItemIsMovable);
}

QRectF GItem::boundingRect() const
{
    // outer most edges
    return QRectF(0,0,size_w,size_h);
}

void GItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    painter->setBrush(Qt::darkGray);

    if(Pressed)
    {
        QPen pen(Color, 3);
        painter->setBrush(Color);
        painter->setPen(pen);
        if (ImageFileName=="")
            painter->drawEllipse(rect);
        else
        {   QImage image(ImageFileName);
            painter->drawImage(rect,image);
        }
    }
    else
    {
        QPen pen(Color, 3);
        painter->setBrush(Color);
        painter->setPen(pen);
        if (ImageFileName=="")
            painter->drawEllipse(rect);
        else
        {   QImage image(ImageFileName);
            painter->drawImage(rect,image);
        }
    }
    painter->drawText(10, size_h - 10, Name);

}

void GItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}

void GItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

