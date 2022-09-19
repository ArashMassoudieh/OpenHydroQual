#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <QDialog>
#include <Wizard_Script.h>
#include <QScrollArea>
#include <QTabWidget>
#include <QFormLayout>

struct tab {
    QScrollArea *scrollArea;
    QFormLayout *formlayout;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QWidget *scrollAreaWidgetContents;
    QFormLayout *formLayout;
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
    void PopulateTab(QWidget *scrollAreaWidgetContents, QFormLayout *formLayout, WizardParameterGroup *paramgroup);
    void PopulateTab(WizardParameterGroup *paramgroup);
    void CreateDummyPage();

private:
    Ui::WizardDialog *ui;
    QMap<QString,tab> tabs;
    WizardScript SelectedWizardScript;

};

#endif // WIZARDDIALOG_H
