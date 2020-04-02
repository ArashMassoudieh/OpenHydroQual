#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>

class DiagramView;

class Node : public QGraphicsItem
{
public:
    Node(DiagramView *parent);

private:
    DiagramView *parent;
};

#endif // NODE_H
