#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsItem>
#include "enums.h"
#include "string"
#include "Object.h"

class Node;
class DiagramView;
class System;


using namespace std;

class Edge : public QGraphicsItem
{
public:
    Edge(DiagramView *parent);
    void adjust();
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    bool bold = false;
    void setBold(const bool _Bold = true);
    objectColor color;
    bool errorDetected() const { return (errors.count()) ? true : false; }
    bool avoidCrossObjects = true;
    void SetObject(Object *_object) {objectPrimaryKey = _object->GetPrimaryKey() ;}
    Object *object();

private:
    QMap<QString, QString> warnings, errors;
    DiagramView *parent;
    Node *source;
    Node *dest;
    System *system;
    string objectPrimaryKey;

public slots:
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event) override;
};


#endif // EDGE_H
