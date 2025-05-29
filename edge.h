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
    Edge(Node *sourceNode, Node *destNode, const QString &edgeType, DiagramView *_parent = nullptr);
    Edge(Node *sourceNode, Node *destNode, DiagramView *_parent=nullptr);
    Edge(Edge &E);
    void adjust();
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
    Object_Types itemType = Object_Types::Connector;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    bool bold = false;
    void setBold(const bool _Bold = true);
    objectColor color;
    bool errorDetected() const { return (errors.count()) ? true : false; }
    bool avoidCrossObjects = true;
    void SetObject(Object *_object) {objectPrimaryKey = _object->GetPrimaryKey() ;}
    Object *object();
    Node* sourceNode() {return source;}
    Node* destNode() {return dest;}
    int dist(const QPointF point);
    QString Name() {return QString::fromStdString(object()->GetName());}
    enum { Type = UserType + 2 };
    int type() const Q_DECL_OVERRIDE{ return Type; }

private:
    QMap<QString, QString> warnings, errors;
    DiagramView *parent;
    Node *source;
    Node *dest;
    System *system;
    string objectPrimaryKey;
    QString name;
    QColor GetColor(const string &clrstring);
public slots:
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};


#endif // EDGE_H
