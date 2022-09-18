#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <QDialog>
#include <Wizard_Script.h>
#include <QScrollArea>

struct tab {
    QScrollArea *scrollArea;
};

namespace Ui {
class WizardDialog;
}

class WizardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizardDialog(QWidget *parent = nullptr);
    void CreateItems(WizardScript *wizscript);
    ~WizardDialog();

private:
    Ui::WizardDialog *ui;
    QMap<QString,tab> tabs;

};

#endif // WIZARDDIALOG_H
