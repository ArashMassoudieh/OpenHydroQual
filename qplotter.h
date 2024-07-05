#ifndef QPLOTTER_H
#define QPLOTTER_H

#include <QtCharts>

QT_BEGIN_NAMESPACE
class QGestureEvent;
QT_END_NAMESPACE

QT_USE_NAMESPACE


class QPlotter : public QChart
{
public:
    explicit QPlotter(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = {});
    ~QPlotter();

protected:
    bool sceneEvent(QEvent *event);

private:
    bool gestureEvent(QGestureEvent *event);

private:

};

#endif // QPLOTTER_H
