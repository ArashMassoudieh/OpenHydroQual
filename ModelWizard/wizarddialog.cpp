#include "wizarddialog.h"
#include "ui_wizarddialog.h"

WizardDialog::WizardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizardDialog)
{
    ui->setupUi(this);
}

WizardDialog::~WizardDialog()
{
    delete ui;

}

void WizardDialog::CreateItems(WizardScript *wizscript)
{
    for (QMap<QString,WizardParameterGroup>::Iterator it=wizscript->GetWizardParameterGroups().begin(); it!=wizscript->GetWizardParameterGroups().end(); it++)
    {
        tab this_tab;
        this_tab.scrollArea = new QScrollArea(this);
        tabs[it->Name()] = this_tab;
        ui->tabWidget->addTab(this_tab.scrollArea,it->Name());
    }
}
