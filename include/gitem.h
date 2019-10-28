#ifndef GITEM_H
#define GITEM_H

#include <QPainter>
#include <QGraphicsItem>

// class for customization

enum class shape { circle, link, pixmap };
enum class corners { none, topleft, topright, bottomleft, bottomright };
enum class Object_Types { Void, Block, Connector, RayLine };

struct props
{
	QColor Color = Qt::black;
	double size_h = 100, size_w = 100;
	QString ImageFileName;
	QString Name;
	QString Info;
	bool show_info;
	shape Shape;
	bool Pressed;
	QString Type;
	int minH = 30, minW = 40; 

};

class GItem :public QGraphicsItem
{
public:
    GItem();
	GItem(const GItem&);
	QPainterPath shape() const Q_DECL_OVERRIDE;
    QRectF boundingRect() const;
    // overriding paint()
    void paint(QPainter * painter,
               const QStyleOptionGraphicsItem * option,
               QWidget * widget);

	props Properties; 
	Object_Types GetItemType() const { return itemType; }
	QString Name() const { return Properties.Name; }
	QString GetType() const { return Properties.Type; }
	int Width() const { return Properties.size_w; }
	int Height() const { return Properties.size_h; }
	void setWidth(int w) { Properties.size_w = w; }
	void setHeight(int h) { Properties.size_h = h; }
	corners corner(const int _x, const int _y, bool absolute);
protected:
    // overriding mouse events
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void hoverMoveEvent(QGraphicsSceneMouseEvent* event);
	
private:
	
	Object_Types itemType = Object_Types::Block;

};

#endif // GITEM_H
