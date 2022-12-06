#include <QRubberBand>
#include "CustomPlotZoom.h"


CustomPlotZoom::CustomPlotZoom(QWidget * parent)
    : QCustomPlot(parent)
    , mZoomMode(true)
    , mRubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{}
 
CustomPlotZoom::~CustomPlotZoom()
{
    delete mRubberBand;
}
 
void CustomPlotZoom::setZoomMode(bool mode)
{
    mZoomMode = mode;
}
 
void CustomPlotZoom::mousePressEvent(QMouseEvent * event)
{
    if (mZoomMode)
    {
        if (event->button() == Qt::LeftButton)
        {
            mOrigin = event->pos();
            mRubberBand->setGeometry(QRect(mOrigin, QSize()));
            mRubberBand->show();
        }
    }
    QCustomPlot::mousePressEvent(event);
}
 
void CustomPlotZoom::mouseMoveEvent(QMouseEvent * event)
{
    if (mRubberBand->isVisible())
    {
        mRubberBand->setGeometry(QRect(mOrigin, event->pos()).normalized());
    }
    //QCustomPlot::mouseMoveEvent(event);
}
void CustomPlotZoom::wheelEvent(QWheelEvent *event)
{
    double scale = pow((double)2, event->angleDelta().y() / 360.0);
	xAxis->scaleRange(scale, xAxis->range().center()); 
	yAxis->scaleRange(scale, yAxis->range().center());	
	replot();

}
void CustomPlotZoom::mouseReleaseEvent(QMouseEvent * event)
{
	if (mRubberBand->isVisible())
	{
		if (mRubberBand->geometry().height() > 5 && mRubberBand->geometry().width() > 5)
		{
			const QRect & zoomRect = mRubberBand->geometry();
			int xp1, yp1, xp2, yp2;
			zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
			auto x1 = xAxis->pixelToCoord(xp1);
			auto x2 = xAxis->pixelToCoord(xp2);
			auto y1 = yAxis->pixelToCoord(yp1);
			auto y2 = yAxis->pixelToCoord(yp2);

			xAxis->setRange(x1, x2);
			yAxis->setRange(y1, y2);
		}
		mRubberBand->hide();
		replot();
	}
	
    QCustomPlot::mouseReleaseEvent(event);
}

void CustomPlotZoom::axisDoubleClick(QCPAxis * axis, QCPAxis::SelectablePart part, QMouseEvent * event)
{
	rescaleAxes(true);
	/*
	if (!graphCount())
		return;
	if (axis == xAxis)
	{
		qreal min = *std::min_element(graph(0)->data()->keys().begin(), graph(0)->data()->keys().end());
		qreal max = *std::max_element(graph(0)->data()->keys().begin(), graph(0)->data()->keys().end());
		for (int i = 0; i < graphCount(); i++)
		{
			if (min > *std::min_element(graph(i)->data()->keys().begin(), graph(i)->data()->keys().end()))
				min = *std::min_element(graph(i)->data()->keys().begin(), graph(i)->data()->keys().end());
			if (max < *std::max_element(graph(i)->data()->keys().begin(), graph(i)->data()->keys().end()))
				max = *std::max_element(graph(i)->data()->keys().begin(), graph(i)->data()->keys().end());
		}
		axis->setRange(min, max);
	}
	else if (axis == yAxis)
	{
		qreal min = *std::min_element(graph(0)->data()->values().begin(), graph(0)->data()->values().end());
		qreal max = *std::max_element(graph(0)->data()->values().begin(), graph(0)->data()->values().end());
		for (int i = 0; i < graphCount(); i++)
		{
			if (min > *std::min_element(graph(i)->data() .begin(), graph(i)->data()->values().end()))
				min = *std::min_element(graph(i)->data()->values().begin(), graph(i)->data()->values().end());
			if (max < *std::max_element(graph(i)->data()->values().begin(), graph(i)->data()->values().end()))
				max = *std::max_element(graph(i)->data()->values().begin(), graph(i)->data()->values().end());
		}
		axis->setRange(min, max);
	}
	axis->rang*/
}
