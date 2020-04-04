#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QWidget>
#include <QGraphicsView>

class MainWindow;
class Node;
class Edge;

class DiagramView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DiagramView(QWidget* parent, MainWindow *_mainwindow);
    QGraphicsScene *MainGraphicsScene;
    MainWindow *mainWindow() {return  mainwindow;}
private:
    MainWindow *mainwindow;
    QList<Node*> nodes;
    QList<Edge*> edges;

signals:

};

#endif // DIAGRAMVIEW_H
