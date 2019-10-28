#include "ray.h"
#include "gitem.h"
#include <QPainter>
#include <math.h>
#include <qdebug.h>
#include <algorithm>

static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;

using namespace std; 

Ray::Ray()
: arrowSize(10)
{
    setAcceptedMouseButtons(0);
	itemType = Object_Types::RayLine;
	GUI = "Ray";

}

void Ray::adjust(GItem *_gitem, QPointF *_Point)
{
    if (!_gitem || !_Point)
        return;
	QLineF line(mapFromItem(_gitem, _gitem->Width() / 2, _gitem->Height() / 2), *_Point);
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
		Ox = line.dx() / abs(line.dx())*min(int(_gitem->Width() / 2), int(fabs(_gitem->Height() / 2.0 * line.dx() / (line.dy()+0.5))));
		Dx = -line.dx();
	}
	if (abs(line.dy()) < 1)
	{
		Oy = 0;
		Dy = 0;
	}
	else
	{
		Oy = line.dy() / abs(line.dy())*min(int(_gitem->Height() / 2), int(fabs(_gitem->Width() / 2.0 * line.dy() / (line.dx()+0.5))));
		Dy = -line.dy();
	}
	
		QPointF RayOffsetSource(Ox, Oy);

		QPointF RayOffsetDest(Dx, Dy);


		sourcePoint = line.p1() + RayOffsetSource;
		destPoint = *_Point;
		update();
}
void Ray::adjust(GItem *source, GItem *dest)
{
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
		Ox = line.dx() / abs(line.dx())*min(int(source->Width() / 2), int(fabs(source->Height() / 2.0 * line.dx() / (line.dy() + 0.5))));
		Dx = -line.dx() / abs(line.dx())*min(int(dest->Width() / 2), int(fabs(dest->Height() / 2.0 * line.dx() / (line.dy() + 0.5))));
	}
	if (abs(line.dy()) < 1)
	{
		Oy = 0;
		Dy = 0;
	}
	else
	{
		Oy = line.dy() / abs(line.dy())*min(int(source->Height() / 2), int(fabs(source->Width() / 2.0 * line.dy() / (line.dx() + 0.5))));
		Dy = -line.dy() / abs(line.dy())*min(int(dest->Height() / 2), int(fabs(dest->Width() / 2.0 * line.dy() / (line.dx() + 0.5))));
	}
	QPointF edgeOffsetSource(Ox, Oy);
	QPointF edgeOffsetDest(Dx, Dy);
	sourcePoint = line.p1() + edgeOffsetSource;
	destPoint = line.p2() + edgeOffsetDest;
}

QRectF Ray::boundingRect() const
{

    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}
void Ray::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
	painter->setPen(QPen(valid?Qt::black:Qt::gray, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(line);

    double angle = ::acos(line.dx() / line.length());
    if (line.dy() >= 0)
        angle = TwoPi - angle;

    QPointF sourceArrowP1 = sourcePoint + QPointF(sin(angle + Pi / 3) * arrowSize,
                                                  cos(angle + Pi / 3) * arrowSize);
    QPointF sourceArrowP2 = sourcePoint + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                  cos(angle + Pi - Pi / 3) * arrowSize);
    QPointF destArrowP1 = destPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                              cos(angle - Pi / 3) * arrowSize);
    QPointF destArrowP2 = destPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                              cos(angle - Pi + Pi / 3) * arrowSize);

	painter->setBrush(valid ? Qt::black : Qt::gray);
//	painter->drawPolygon(QPolygonF() << line.p1() << sourceArrowP1 << sourceArrowP2);
    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
}
void Ray::setValidation(bool _valid)
{
	valid = _valid;
}
