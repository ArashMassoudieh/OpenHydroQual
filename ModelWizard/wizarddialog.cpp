#include "wizarddialog.h"
#include "ui_wizarddialog.h"
#include <QLabel>
#include <QLineEdit>
#include "UnitTextBox3.h"
#include <QSpinBox>


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
    SelectedWizardScript = *wizscript;
    for (QMap<QString,WizardParameterGroup>::Iterator it=wizscript->GetWizardParameterGroups().begin(); it!=wizscript->GetWizardParameterGroups().end(); it++)
    {
        tab this_tab;
        this_tab.tab = new QWidget();
        this_tab.tab->setObjectName(it->Name());

        this_tab.horizontalLayout = new QHBoxLayout(this_tab.tab);
        this_tab.horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        this_tab.scrollArea = new QScrollArea(this_tab.tab);
        this_tab.scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        this_tab.scrollArea->setWidgetResizable(true);
        this_tab.scrollAreaWidgetContents = new QWidget();
        this_tab.scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        this_tab.scrollAreaWidgetContents->setGeometry(QRect(0, 0, 528, 308));
        this_tab.formLayout = new QFormLayout(this_tab.scrollAreaWidgetContents);
        this_tab.formLayout->setObjectName(it->Description());


        this_tab.scrollArea->setWidget(this_tab.scrollAreaWidgetContents);

        this_tab.horizontalLayout->addWidget(this_tab.scrollArea);

        ui->tabWidget->addTab(this_tab.tab,it->Description());

        ui->tabWidget->setTabText(ui->tabWidget->indexOf(this_tab.tab), it->Description());
        tabs[it->Name()] = this_tab;
        //PopulateTab(this_tab.scrollAreaWidgetContents,this_tab.formLayout,&it.value());
        PopulateTab(&it.value());


    }
}

void WizardDialog::PopulateTab(QWidget *scrollAreaWidgetContents, QFormLayout *formLayout, WizardParameterGroup *paramgroup)
{
    for (int i=0; i<paramgroup->ParametersCount(); i++)
    {
        QLabel *label = new QLabel(scrollAreaWidgetContents);
        label->setObjectName(paramgroup->Parameter(i)+"_label");
        label->setText(SelectedWizardScript.GetWizardParameters()[paramgroup->Parameter(i)].Question());

        QLineEdit *lineEdit = new QLineEdit(scrollAreaWidgetContents);
        lineEdit->setObjectName(paramgroup->Parameter(i)+"_lineedit");
        formLayout->addRow(label,lineEdit);


    }
}

void WizardDialog::PopulateTab(WizardParameterGroup *paramgroup)
{
    tab this_tab = tabs[paramgroup->Name()];
    for (int i=0; i<paramgroup->ParametersCount(); i++)
    {
        WizardParameter *parameter = &SelectedWizardScript.GetWizardParameters()[paramgroup->Parameter(i)];
        QLabel *label = new QLabel(this_tab.scrollAreaWidgetContents);
        label->setObjectName(paramgroup->Parameter(i)+"_label");
        label->setText(parameter->Question());
        if (parameter->Delegate()=="ValueBox")
        {   QLineEdit *Editor = new QLineEdit(this_tab.scrollAreaWidgetContents);
            Editor->setObjectName(paramgroup->Parameter(i)+"_edit");
            this_tab.formLayout->addRow(label,Editor);
        }
        else if (parameter->Delegate()=="UnitBox")
        {   UnitTextBox3 *Editor = new UnitTextBox3(QStyleOptionViewItem(),this_tab.scrollAreaWidgetContents);
            Editor->setUnitsList(parameter->Units());
            Editor->setObjectName(paramgroup->Parameter(i)+"_edit");
            this_tab.formLayout->addRow(label,Editor);
        }
        else if (parameter->Delegate()=="SpinBox")
        {
            QSpinBox *Editor = new QSpinBox(this_tab.scrollAreaWidgetContents);
            Editor->setObjectName(paramgroup->Parameter(i)+"_edit");
            Editor->setRange(parameter->Range()[0],parameter->Range()[1]);
            this_tab.formLayout->addRow(label,Editor);
        }


    }
}

void WizardDialog::CreateDummyPage()
{

}
