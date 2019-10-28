#pragma once
#include <QGraphicsItem>
#include "gitem.h"
//#include "node.h"

class Node;

class Ray : public QGraphicsItem
{
public:
	Ray();
//	Ray(Node *sourceNode, Node *destNode);
//	Ray(const Ray &);
//	Ray Ray::operator=(const Ray &);
//    Node *sourceNode() const;
//    Node *destNode() const;
	QString Name, ID;
	Object_Types itemType;
    void adjust(GItem *_gitem, QPointF *_Point);
	void adjust(GItem *source, GItem *dest);

//    enum { Type = UserType + 2 };
//    int type() const Q_DECL_OVERRIDE { return Type; }
	void setValidation(bool _valid);
	bool valid = false;
	QString GUI;

protected:
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
 //   Node *source, *dest;
	int sourceID, destID;
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
};
