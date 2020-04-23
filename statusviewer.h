#ifndef STATUSVIEWER_H
#define STATUSVIEWER_H
#include <QLabel>


class QTimer;

class StatusViewer : public QLabel
{
    Q_OBJECT
public:
    StatusViewer(QWidget* p = 0);

    void showInfo(const QString& msg, int delay = -1);
    void showWarn(const QString& msg, int delay = -1);
    void showError(const QString& msg, int delay = -1);

private:
    void setStatus(const QString& msg, int delay, const QColor& color);

private slots:
    void onTimeout();

private:
    QTimer* _timer;
};

#endif // STATUSVIEWER_H
