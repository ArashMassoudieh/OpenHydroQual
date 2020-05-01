#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include "enums.h"
#include "ray.h"
#include "Block.h"

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
    void setconnectfeature(QString cf) {connect_feature = cf;} //sets the type of connector to be used
    Operation_Modes setMode(Operation_Modes OMode = Operation_Modes::NormalMode, bool back = false);
    Operation_Modes setModeCursor();
    void nodeContextMenuRequested(Node* ,QPointF pos, QMenu *menu=nullptr);
    void edgeContextMenuRequested(Edge*, QPointF pos, QMenu *menu=nullptr);
    Node* node(const QString &name) const;
    Edge* edge(const QString &name) const;
    void scaleView(qreal scaleFactor);
    void DeleteAllItems();

private:
    MainWindow *mainwindow;
    QList<Node*> nodes();
    QList<Edge*> edges();
    QString connect_feature = "";
    Operation_Modes Operation_Mode;
    Node *resizenode;
    corners resizecorner;
    Node *Node1; // , *Node2;
    Ray *tempRay;
    int _x, _y;
    QList<Node*> nodes(const QList<QGraphicsItem*> items) const;
    QList<Edge*> edges(const QList<QGraphicsItem*>items) const;
    QList<Node*> selectedNodes() const;
    QList<Edge*> selectedEdges() const;
    QList<Node*> Nodes() const;
    QList<Edge *> Edges() const;
    Operation_Modes setMode(int i);
    QMap<QString, QMap<QString, QString>> specs;
    void updateNodeCoordinates();
    void sceneChanged();
    QString nodenametobedeleted = ""; 
    QString timeseriestobeshown = ""; 
    QString nodenametobecopied = "";
    Block copied_block; 
signals:
    void Mouse_Pos(int, int, QString);
    void changed();
public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void deleteselectednode(QString nodename="");
    void copyselectednode(QString nodename = "");
    void showgraph();


};

#endif // DIAGRAMVIEW_H
