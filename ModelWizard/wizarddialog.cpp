#include "wizarddialog.h"
#include "ui_wizarddialog.h"
#include <QLabel>
#include <QLineEdit>
#include "UnitTextBox3.h"
#include <QSpinBox>
#include <QStandardPaths>
#include <QDateEdit>
 


WizardDialog::WizardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizardDialog)
{
    ui->setupUi(this);
    ui->Previous->setEnabled(false);
    connect(ui->Next, SIGNAL(clicked()),this, SLOT(on_next_clicked()));
    connect(ui->Previous, SIGNAL(clicked()),this, SLOT(on_previous_clicked()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(on_TabChanged()));

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
    on_TabChanged();
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
        if (parameter->Delegate() != "")
        {
            QLabel* label = new QLabel(this_tab.scrollAreaWidgetContents);
            label->setObjectName(paramgroup->Parameter(i) + "_label");
            label->setText(parameter->Question());
            if (parameter->Delegate() == "ValueBox")
            {
                QLineEdit* Editor = new QLineEdit(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "UnitBox")
            {
                UnitTextBox3* Editor = new UnitTextBox3(QStyleOptionViewItem(), this_tab.scrollAreaWidgetContents);
                Editor->setUnitsList(parameter->Units());
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->setMinimumHeight(30);
                Editor->setMinimumWidth(100);
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "SpinBox")
            {
                QSpinBox* Editor = new QSpinBox(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->setRange(parameter->Range()[0], parameter->Range()[1]);
                Editor->setMinimumWidth(100);
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "ComboBox")
            {
                QComboBox* Editor = new QComboBox(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->addItems(parameter->ComboItems());
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "DateBox")
            {
                QDateEdit* Editor = new QDateEdit(this_tab.scrollAreaWidgetContents);
                Editor->setDisplayFormat("MM.dd.yyyy");
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "FileBrowser")
            {
                QPushButton* Editor = new QPushButton(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
        }
    }
}

void WizardDialog::on_next_clicked()
{
    if (ui->tabWidget->currentIndex()<ui->tabWidget->count()-1)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);
    else
    {
        GenerateModel();
    }
    on_TabChanged();
}
void WizardDialog::on_previous_clicked()
{
    if (ui->tabWidget->currentIndex()>0)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()-1);

    on_TabChanged();
}

void  WizardDialog::on_TabChanged()
{
    if (ui->tabWidget->currentIndex()==0)
        ui->Previous->setEnabled(false);
    else
        ui->Previous->setEnabled(true);
    if (ui->tabWidget->currentIndex()==ui->tabWidget->count()-1)
        ui->Next->setText("Create Model");
    else
        ui->Next->setText("Next");

}

void WizardDialog::GenerateModel()
{
    for (QMap<QString,WizardParameter>::iterator it=SelectedWizardScript.GetWizardParameters().begin(); it!=SelectedWizardScript.GetWizardParameters().end(); it++)
    {
        if (it.value().Delegate()=="ValueBox")
            it.value().SetValue(static_cast< QLineEdit*>(it.value().EntryItem())->text());
        else if (it.value().Delegate()=="UnitBox")
            it.value().SetValue(static_cast< UnitTextBox3*>(it.value().EntryItem())->text());
        else if (it.value().Delegate()=="SpinBox")
            it.value().SetValue(static_cast<QSpinBox*>(it.value().EntryItem())->text());
        else if (it.value().Delegate()=="ComboBox")
            it.value().SetValue(static_cast<QComboBox*>(it.value().EntryItem())->currentText());
        else if (it.value().Delegate() == "DateBox")
            it.value().SetValue(static_cast<QDateEdit*>(it.value().EntryItem())->text());
        else if (it.value().Delegate() == "FileBrowser")
            it.value().SetValue(static_cast<QPushButton*>(it.value().EntryItem())->text());
        


    }
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OpenHydroQual files (*.ohq)"),nullptr,QFileDialog::DontUseNativeDialog);
    QStringList script = SelectedWizardScript.Script();

}

