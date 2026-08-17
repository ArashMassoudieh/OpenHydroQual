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


#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include "enums.h"
#include "ray.h"
#include "Block.h"
#include "Composite.h"

class MainWindow;
class Node;
class Edge;

struct colorCodeData
{
    bool nodes = false, edges = false;
};

class DiagramView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DiagramView(QWidget* parent, MainWindow *_mainwindow);
    QGraphicsScene *MainGraphicsScene;
    MainWindow *mainWindow() {return  mainwindow;}
    colorCodeData colorCode;
    QString getselectedconnectfeature() {return connect_feature;} //returns the type of connector to be added.
    void setconnectfeature(QString cf); //sets the type of connector to be used
    /**
     * @brief Abandons a half drawn link and returns to normal mode
     * @param reason shown in the status bar; empty just clears the message
     */
    void CancelLinkDrawing(const QString &reason = QString());
    /** @brief Starts a link at @p source and follows the cursor until release */
    void BeginLinkFrom(Node *source);
    Operation_Modes setMode(Operation_Modes OMode = Operation_Modes::NormalMode, bool back = false);
    Operation_Modes setModeCursor();
    Operation_Modes GetOperationMode() {return Operation_Mode;}
    void nodeContextMenuRequested(Node* ,QPointF pos, QMenu *menu=nullptr);
    void edgeContextMenuRequested(Edge*, QPointF pos, QMenu *menu=nullptr);
    Node* node(const QString &name) const;
    Edge* edge(const QString &name) const;
    void scaleView(qreal scaleFactor);
    void DeleteAllItems();
    void wheelEvent(QWheelEvent* pWheelEvent) override;
    double fontfactor=1;
    int linkthickness = 1;
    bool showlinkicons = false;
    QList<Node*> Nodes() const;
    QList<Edge *> Edges() const;
    void UnSelectAll();
    void UpdateSceneRect();
private:
    MainWindow *mainwindow;
    QList<Node*> nodes();
    QList<Edge*> edges();
    QString connect_feature = "";
    Operation_Modes Operation_Mode;
    Node *resizenode;
    corners resizecorner;
    Node *Node1 = nullptr; // , *Node2;
    Ray *tempRay = nullptr;
    /**
     * @brief Cursor showing the icon of the link type being drawn
     *
     * Built when a link tool is armed, so the pointer itself says which link is about to
     * be drawn instead of a generic crosshair.
     */
    QCursor linkCursor = QCursor(Qt::CrossCursor);
    /**
     * @brief Picks one item out of those under the cursor, preferring blocks over links
     *
     * This used to pick at random, which meant a click where a link crossed a block
     * started a drag on whichever the coin toss chose.
     */
    static void PickNodeOrEdge(const QList<QGraphicsItem*> &items, Node *&node, Edge *&edge);
    int _x, _y;
    int x_ini, y_ini;
    QList<Node*> nodes(const QList<QGraphicsItem*> items) const;
    QList<Edge*> edges(const QList<QGraphicsItem*>items) const;
    QList<Node*> selectedNodes() const;
    QList<Edge*> selectedEdges() const;
    Operation_Modes setMode(int i);
    QMap<QString, QMap<QString, QString>> specs;
    void updateNodeCoordinates();
    void sceneChanged();
    QString nodenametobedeleted = ""; 
    QString timeseriestobeshown = ""; 
    QString nodenametobecopied = "";
    Block copied_block;
    Composite copied_composite;
    bool copied_is_composite = false;
    bool RecreateGraphics = false; 
signals:
    void Mouse_Pos(int, int, QString);
    void changed();
public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool deleteselectednode(QString nodename="");
    void copyselectednode(QString nodename = "");
    void showgraph();
    void pastecopieddnode(); 


};

#endif // DIAGRAMVIEW_H
