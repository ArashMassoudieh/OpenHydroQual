#ifndef NODE_H
#define NODE_H

#include "enums.h"
#include <QGraphicsItem>

class DiagramView;
class System;
class QuanSet;
class Object;

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
    void SetObject(Object *_object) {object = _object;}
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


private:
    DiagramView *parent;
    System *system;
    int width = 200;
    int height = 200;
    QString name;
    QString iconfilename;
    QPointF newPos;
    bool bold = false;
    Object* object;
};

#endif // NODE_H
