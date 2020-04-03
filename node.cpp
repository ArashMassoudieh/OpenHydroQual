#include "node.h"
#include "diagramview.h"
#include "System.h"



Node::Node(DiagramView *_parent, System *_system)
{
    parent = _parent;
    system = _system;

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);

    itemType = Object_Types::BlockType;
    //setX(x);
    //setY(y);

    //width = max(minW, _width);
    //height = max(minH, _height);

    parent->MainGraphicsScene->addItem(this);


}
