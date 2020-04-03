#ifndef NODE_H
#define NODE_H

#include "enums.h"
#include <QGraphicsItem>

class DiagramView;
class System;

enum Object_Types {BlockType, EdgeType};

class Node : public QGraphicsItem
{
public:
    Node(DiagramView *parent, System *_system);
    Object_Types itemType;
    Node(const Node &);
    objectColor color;
    Node operator=(const Node &);
    enum { Type = UserType + 1 };
    int type() const Q_DECL_OVERRIDE { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;


private:
    DiagramView *parent;
    System *system;
};

#endif // NODE_H
