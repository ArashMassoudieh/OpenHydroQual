#include "AquiGraphicsView.h"
#include "mainwindow.h"
#include "QMouseEvent"
#include "gitem.h"

AquiGraphicsView::AquiGraphicsView(QWidget* _parent, MainWindow* mainWindow)
{
	parent = _parent;
	this->mainWindow = mainWindow;
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	Operation_Mode = Operation_Modes::NormalMode; 
}

void AquiGraphicsView::mousePressEvent(QMouseEvent* event)
{
	Node* node = qgraphicsitem_cast<Node*> (itemAt(event->pos())); //Get the item at the position
	if (node)
	{	//qDebug() << "Name: "<< node->Name()<<" Flag:" << node->flags() << "enabled:" << node->isEnabled() << "active:" << node->isActive();
	}
	//Edge* edge = qgraphicsitem_cast<Edge*> (itemAt(event->pos())); //Get the item at the position Arash Pay attention
	//if (edge)
	//{	//qDebug() << "Name: " << edge->Name() << " Flag:" << edge->flags() << "enabled:" << edge->isEnabled() << "active:" << edge->isActive();
	//}

	if (event->buttons() == Qt::MiddleButton && Operation_Mode == Operation_Modes::NormalMode)
	{
		setMode(Operation_Modes::Pan);
		QMouseEvent* newEvent = new QMouseEvent(event->type(), event->localPos(), Qt::LeftButton, Qt::MouseButtons(1), event->modifiers());
		QGraphicsView::mousePressEvent(newEvent);
		delete newEvent;
		return;
	}
	QGraphicsView::mousePressEvent(event); //Call the ancestor
	if (Operation_Mode == Operation_Modes::NormalMode)
	{
		setDragMode((node || edge) ? QGraphicsView::NoDrag : QGraphicsView::RubberBandDrag);

		if (node)
			if (node->itemType == Object_Types::Block)
			{
				int xx = mapToScene(event->pos()).x();	int yy = mapToScene(event->pos()).y();
				if (event->buttons() == Qt::LeftButton)
				{
					if (event->modifiers() && Qt::ControlModifier) {
						//						node->setFlag(QGraphicsItem::ItemIsMovable, false);
						//						node->setSelected(true);
					}
					else if (node->corner(xx, yy)) {
						setDragMode(QGraphicsView::NoDrag);
						setMode(Operation_Modes::resizeNode);
						resizenode = node;
						resizecorner = node->corner(xx, yy);
						node->setFlag(QGraphicsItem::ItemIsMovable, false);
					}
					else if (node->edge(xx, yy)) {
						node->setFlag(QGraphicsItem::ItemIsMovable, false);
						Node1 = node;
						tempRay = new Ray();
						MainGraphicsScene->addItem(tempRay);
						setMode(Operation_Modes::Node1_selected);
					}
					else
						node->setFlag(QGraphicsItem::ItemIsMovable, true);
				}
			}
		if (event->buttons() == Qt::LeftButton && Operation_Mode == Operation_Modes::Draw_Connector)
		{
			if (node) {
				if (node->itemType == Object_Types::Block) {
					node->setFlag(QGraphicsItem::ItemIsMovable, false);
					Node1 = node;
					tempRay = new Ray();
					MainGraphicsScene->addItem(tempRay);
					setMode(Operation_Modes::Node1_selected);
				}
			}
		}
	}

	//QGraphicsView::mousePressEvent(event);
}


void AquiGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
	double x = 0; 
	_x = mapToScene(event->pos()).x();
	_y = mapToScene(event->pos()).y();
	int xx = _x;// mapToScene(event->pos()).x();
	int yy = _y;// mapToScene(event->pos()).y();

	if (event->buttons() == Qt::MiddleButton && Operation_Mode == Operation_Modes::Pan)
	{
		QMouseEvent* newEvent = new QMouseEvent(event->type(), event->localPos(), Qt::LeftButton, Qt::MouseButtons(1), event->modifiers());
		//		QGraphicsView::mousePressEvent(newEvent);
		QGraphicsView::mouseMoveEvent(newEvent);
		delete newEvent;
		return;
	}

	bool cursorModeNormal = true;
	setToolTip("");
	QGraphicsView::mouseMoveEvent(event);
	QString txt;
	GItem* item1 = qgraphicsitem_cast<GItem*> (itemAt(event->pos())); //Get the item at the position
	if (item1) //itemType == Object_Types::Block)
	{
		txt = item1->Properties.Name + "\n" + item1->Properties.Info;
		QString toolTip = QString(txt);
		toolTip.append(txt);
		setToolTip(toolTip);
	}
	
	emit Mouse_Pos(_x, _y, txt);
	if (Operation_Mode == Operation_Modes::Node1_selected)
	{
		GItem* child = qgraphicsitem_cast<GItem*> (itemAt(event->pos())); //Get the item at the position
		if (child)
			if ((child->GetItemType() == Object_Types::Block) && (item1->Name() != child->Name()))
			{
				tempRay->setValidation(true);
				tempRay->adjust(item1, child);
			}
		if (!child)
		{
			tempRay->setValidation(false);
			QPointF p = QPointF(mapToScene(event->pos()));
			tempRay->adjust(item1, &p);
		}
	}

	if (Operation_Mode == Operation_Modes::NormalMode && dragMode() == DragMode::NoDrag)
	{
		
		GItem* item = qgraphicsitem_cast<GItem*> (itemAt(event->pos())); //Get the item at the position
																	   /*		Edge *edge = static_cast<Edge*> (itemAt(event->pos())); //Get the item at the position
																	   if (edge)
																	   {
																	   setCursor(Qt::ForbiddenCursor);
																	   //qDebug() << edge->dist(mapToScene(event->pos()));
																	   }
																	   */
		if (item)
			if (item->GetItemType() == Object_Types::Block)
			{
				if ((item->corner(xx, yy, true) == corners::topleft) || (item->corner(xx, yy, true) == corners::bottomright))
				{
					setCursor(Qt::SizeFDiagCursor);
					cursorModeNormal = false;
				}
				if ((item->corner(xx, yy,true) == corners::topright) || (item->corner(xx, yy, true) == corners::bottomleft))
				{
					setCursor(Qt::SizeBDiagCursor);
					cursorModeNormal = false;
				}
			}
	}

	if ((Operation_Mode == Operation_Modes::resizeNode) && (event->buttons() == Qt::LeftButton))
	{
		int xx = mapToScene(event->pos()).x();
		int yy = mapToScene(event->pos()).y();
		int px = resizenode->x();
		int py = resizenode->y();
		int pw = resizenode->Width();
		int ph = resizenode->Height();
		int minH = resizenode->Properties.minH, minW = resizenode->Properties.minW;

		if (resizecorner == corners::topleft && (px - xx + pw) > minW && (py - yy + ph) > minH)
		{
			resizenode->setX(xx);
			resizenode->setWidth(px - xx + pw);
			resizenode->setY(yy);
			resizenode->setHeight(py - yy + ph);
		}
		if (resizecorner == corners::bottomleft && (px - xx + pw) > minW && (yy - py) > minH)
		{
			resizenode->setX(xx);
			resizenode->setWidth(px - xx + pw);
			//resizenode->setY(yy);
			resizenode->setHeight(yy - py);
		}
		if (resizecorner == corners::topright && (xx - px) > minW && (py - yy + ph) > minH)
		{
			//resizenode->setX(xx);
			resizenode->setWidth(xx - px);
			resizenode->setY(yy);
			resizenode->setHeight(py - yy + ph);
		}
		if (resizecorner == corners::bottomright && (xx - px) > minW && (yy - py) > minH)
		{
			//resizenode->setX(xx);
			resizenode->setWidth(xx - px);
			//resizenode->setY(yy);
			resizenode->setHeight(yy - py);
		}
		resizenode->update();
		//for (Edge* edge : resizenode->edges()) Arash Pay Attention
		//	edge->adjust();
	}
	if (cursorModeNormal)
		setCursor(Qt::ArrowCursor);

	QGraphicsView::mouseMoveEvent(event);
}

QList<GItem*> AquiGraphicsView::Items() const
{
	QList<GItem*> items;
	foreach(QGraphicsItem * item, scene()->items()) {
		if (GItem* item = qgraphicsitem_cast<GItem*>(item))
		{
			items << item; 
		}
	}
	return(items);
}