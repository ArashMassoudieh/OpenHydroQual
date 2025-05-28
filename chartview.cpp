/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#include "chartview.h"
#include <QtGui/QMouseEvent>
#include "mainwindow.h"
#include "qplotwindow.h"

ChartView::ChartView(QChart *_chart, QPlotWindow *_plotWindow, MainWindow *_parent) :
    QChartView(_chart, _parent),
    m_isTouching(false)
{
    parent = _parent;
    plotWindow = _plotWindow;
    setRubberBand(QChartView::NoRubberBand);
}

bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        m_isTouching = true;

        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    qDebug()<<event->type();
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        chart()->zoomReset();
        return;
    }
    if (m_isTouching)
        return;

    if (event->button() == Qt::MiddleButton)
    {
        m_lastMousePos = event->pos();
        event->accept();
    }



    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isTouching)
        return;

    if (event->buttons() == Qt::MiddleButton)
    {
        auto dPos = event->pos() - m_lastMousePos;
        chart()->scroll(-dPos.x(), dPos.y());

        m_lastMousePos = event->pos();
        event->accept();

    }

    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (double_clicked)
    {
        double_clicked = false;
        return;
    }

    if (m_isTouching)
        m_isTouching = false;

    chart()->setAnimationOptions(QChart::SeriesAnimations);

    if (event->button() == Qt::MouseButton::LeftButton)
    {
        QRect rect(QPoint(m_lastMousePos.x(),m_lastMousePos.y()), QPoint(event->pos().x(),event->pos().y()));
        chart()->zoomIn(rect);
    }
    else
    {
        QMenu *menu = new QMenu(this);
        if (chart()->series().count()==1)
        {
            menu->addAction("Copy curve");
        }
        else
            menu->addAction("Copy Curves");
        QAction *paste = menu->addAction("Paste");
        if (parent->graphsClipboard.isEmpty())
            paste->setEnabled(false);
        else
            paste->setEnabled(true);

        menu->addAction("Zoom Extends");
        QMenu *subMenuY = menu->addMenu("Y-axis");
        QAction *Ylog_action = subMenuY->addAction("Log-scale");
        Ylog_action->setCheckable(true);
        Ylog_action->setChecked(Ylog());
        QAction *selectedAction = menu->exec(mapToGlobal(event->pos()));
        if (!selectedAction)
            return;
        if (selectedAction->text().contains("Copy"))
        {
            parent ->graphsClipboard.clear();
            for (int i = 0; i < chart()->series().count() ; i++)
            {
                parent->graphsClipboard.insert(chart()->series()[i]->name(),  new CTimeSeries<double>(plotWindow->GetTimeSeries(chart()->series()[i]->name())) );
            }

        }
        if (selectedAction->text().contains("Paste"))
        {
            for (QMap<QString, CTimeSeries<double>*>::Iterator it = parent->graphsClipboard.begin(); it!= parent->graphsClipboard.end(); it++)
                plotWindow->AddData(*it.value());

        }
        if (selectedAction->text().contains("Zoom Extends"))
        {
            chart()->zoomReset();

        }
        if (selectedAction->text().contains("Log"))
        {
            SetYLogAxis(!Ylog());
            if (Ylog())
            {
                Ylog_action->setChecked(true);
            }
            else
                Ylog_action->setChecked(false);

        }

    }

}


void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    case Qt::Key_Space:
        chart()->zoomReset();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        chart()->zoomReset();
        double_clicked = true;
    }

    QGraphicsView::mouseDoubleClickEvent( e );
}

void ChartView::SetYLogAxis(bool val) {

    logYAxis = val;
    plotWindow->SetYAxis(val);
}
