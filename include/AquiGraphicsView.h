#pragma once
#include "qgraphicsview.h"
#include "gitem.h"
#include "ray.h"

class MainWindow; 



enum class Operation_Modes { Draw_Connector, Node1_selected, Pan, NormalMode, resizeNode };

class AquiGraphicsView :
	public QGraphicsView
{
	Q_OBJECT
public:
	AquiGraphicsView(QWidget* parent, MainWindow* mainWindow);
	QWidget* parent;
	MainWindow* mainWindow;
	QList<GItem*> Items() const;

protected:
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
private:
	int _x, _y;
	Operation_Modes Operation_Mode;
	Ray* tempRay;
	GItem* resizenode; 
	corners resizecorner; 
signals:
	void Mouse_Pos(int, int, QString);
	void changed();
	
};

