/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#ifndef NODE_H
#define NODE_H

#include "enums.h"
#include <QGraphicsItem>
#include <string>


class DiagramView;
class System;
class QuanSet;
class Object;
class Edge;

using namespace std;
class Node : public QGraphicsItem
{
public:
    Node(DiagramView *parent, System *_system);
    Object_Types itemType = Object_Types::Block;
    Node(const Node &);
    objectColor color;
    Node operator=(const Node &);
    enum { Type = UserType + 1 };
    int type() const Q_DECL_OVERRIDE { return Type; }
    void SetObject(Object* _object);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    Object *object();
    int Width() const {return  width;}
    int Height() const {return height;}
    corners corner(const int _x, const int _y);
    edgesides edge(const int x, const int y);
    QString Name();
    int minH = 30, minW = 40;
    void setWidth(const int& Width);
    void setHeight(const int& Height);
    void setX(const int& x);
    void setY(const int& y);
    double fontfactor();
    QList<Edge *> edges() const { return edgeList; }
    void addEdge(Edge *edge);

private:
    DiagramView *parent;
    System *system;
    int width = 200;
    int height = 200;
    QString name;
    QString iconfilename;
    QPointF newPos;
    bool bold = false;
    string objectPrimaryKey;
    QList<Edge *> edgeList;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;


signals:
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event) override;

};

#endif // NODE_H
