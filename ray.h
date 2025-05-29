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


#ifndef RAY_H
#define RAY_H
#pragma once
#include <QGraphicsItem>
#include <enums.h>
//#include "node.h"

class Node;

class Ray : public QGraphicsItem
{
public:
    Ray();
    QString Name, ID;
    Object_Types itemType;
    void adjust(Node *_Node, QPointF *_Point);
    void adjust(Node *source, Node *dest);

    void setValidation(bool _valid);
    bool valid = false;
    QString GUI;

protected:
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:

    int sourceID, destID;
    QPointF sourcePoint;
    QPointF destPoint;
    qreal arrowSize = 10;
};

#endif // RAY_H
