#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsItem>

class DiagramView;

class Edge : public QGraphicsItem
{
public:
    Edge(DiagramView *parent);

private:
    DiagramView *parent;
};


#endif // EDGE_H
