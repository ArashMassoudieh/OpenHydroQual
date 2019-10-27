#ifndef GITEM_H
#define GITEM_H

#include <QPainter>
#include <QGraphicsItem>

// class for customization
class GItem :public QGraphicsItem
{
public:
    GItem();
    enum class shape {circle, link, pixmap};

    shape Shape;
    QColor Color = Qt::black;
    QRectF boundingRect() const;
    double size_h = 100, size_w = 100;
    QString ImageFileName;
    QString Name;
    QString Info;
    bool show_info;
    // overriding paint()
    void paint(QPainter * painter,
               const QStyleOptionGraphicsItem * option,
               QWidget * widget);

    // item state
    bool Pressed;
protected:
    // overriding mouse events
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // GITEM_H
