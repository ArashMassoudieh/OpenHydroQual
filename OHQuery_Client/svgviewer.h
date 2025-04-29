#ifndef SVGViewer_H
#define SVGViewer_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>

class SVGViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SVGViewer(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent* event) override;
signals:
    // Custom signal emitted on double-click
    void doubleClicked(const QPointF &scenePos);

protected:
    // Override the mouseDoubleClickEvent
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif // SVGViewer_H
