#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    void AppendText(const QString &text);
    void ClearText();
private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
