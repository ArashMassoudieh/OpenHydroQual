#include "diagramview.h"
#include "mainwindow.h"

DiagramView::DiagramView(QWidget* parent, MainWindow *_mainwindow) : QGraphicsView(parent)
{
    mainwindow = _mainwindow;
    MainGraphicsScene = new QGraphicsScene(this);
    MainGraphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    MainGraphicsScene->setSceneRect(0, 0, 2000, 2000);
    setScene(MainGraphicsScene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    QObject::connect(MainGraphicsScene, SIGNAL(changed(const QList<QRectF>)), this, SLOT(sceneChanged()));

}
