#include "gitem.h"
#include <QGraphicsSceneMouseEvent>
#include <qcursor.h>

GItem::GItem()
{
    Properties.Pressed = false;
    setFlag(ItemIsMovable);
	setAcceptHoverEvents(true);
	
}

GItem::GItem(const GItem& G)
{
	Properties = G.Properties; 
}

QPainterPath GItem::shape() const
{
	QPainterPath path;
	path.addRect(boundingRect());
	return path;
}

QRectF GItem::boundingRect() const
{
    return QRectF(0,0,Properties.size_w,Properties.size_h);
}

void GItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    painter->setBrush(Qt::darkGray);

    if(Properties.Pressed)
    {
        QPen pen(Properties.Color, 3);
        painter->setBrush(Properties.Color);
        painter->setPen(pen);
        if (Properties.ImageFileName=="")
            painter->drawEllipse(rect);
        else
        {   QImage image(Properties.ImageFileName);
            painter->drawImage(rect,image);
        }
    }
    else
    {
        QPen pen(Properties.Color, 3);
        painter->setBrush(Properties.Color);
        painter->setPen(pen);
        if (Properties.ImageFileName=="")
            painter->drawEllipse(rect);
        else
        {   QImage image(Properties.ImageFileName);
			painter->setOpacity(0.5);
			painter->drawImage(rect,image);
        }
    }
    painter->drawText(Properties.size_w/2, Properties.size_h/2, Properties.Name);

}

void GItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	
	Properties.Pressed = true;
    update();
    QGraphicsItem::mousePressEvent(event);
}

void GItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Properties.Pressed = false;
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}

void GItem::hoverMoveEvent(QGraphicsSceneMouseEvent* event)
{
	double x = event->pos().x();
	double y = event->pos().y();
	if (corner(event->pos().x(), event->pos().y(), false) != corners::none)
	{
		setCursor(Qt::PointingHandCursor);
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}
}
corners GItem::corner(const int _x, const int _y, bool absolute)
{
	double xx = 0; 
	double yy = 0; 
	if (absolute)
	{
		double xx = x();
		double yy = y();
	}
	
	int border = 15;
	if (abs(_x - xx) < border)
	{
		if (abs(_y - yy) < border) return (corners::topleft);
		if (abs(_y - yy - Properties.size_h) < border) return (corners::bottomleft);
	}
	if (abs(_x - xx - Properties.size_w) < border)
	{
		if (abs(_y - yy) < border) return (corners::topright);
		if (abs(_y - yy - Properties.size_h) < border) return (corners::bottomright);
	}
	return (corners::none);
}
