#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

class MainWindow;

namespace Ui {
class Dialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(MainWindow *parent = nullptr);
    ~OptionsDialog();

private:
    Ui::Dialog *ui;
    MainWindow *parent = nullptr;

private slots:
    void on_ok_clicked();
    void on_cancel_clicked();
};

#endif // OPTIONS_H
