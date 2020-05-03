#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>

namespace Ui {
class logwindow;
}

class logwindow : public QDialog
{
    Q_OBJECT

public:
    explicit logwindow(QWidget *parent = nullptr);
    ~logwindow();
    void AppendText(const QString &s);
    void AppendError(const QString &s);
    void AppendBlue(const QString &s);

private:
    Ui::logwindow *ui;
};

#endif // LOGWINDOW_H
