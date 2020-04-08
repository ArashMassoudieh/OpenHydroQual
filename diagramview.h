#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QWidget>
#include <QGraphicsView>

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
private:
    MainWindow *mainwindow;
    QList<Node*> nodes;
    QList<Edge*> edges;
    QString connect_feature = "";
signals:

public slots:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

};

#endif // DIAGRAMVIEW_H
