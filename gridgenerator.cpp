#include "gridgenerator.h"
#include "ui_gridgenerator.h"
#include "mainwindow.h"
#include "UnitTextBox3.h"
#include "utilityfuncs.h"
#include "expEditor.h"
#include "node.h"
#include "edge.h"

GridGenerator::GridGenerator(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::GridGenerator)
{
    ui->setupUi(this);
    mainwindow = parent;
    PopulateBlocks();
    PopulateLinks(); 
    connect(ui->listWidgetBlocks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_block_changed()));
    connect(ui->listWidgetLinks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_link_changed()));
    connect(ui->pushButtonNext,SIGNAL(clicked()),this,SLOT(on_NextClicked()));
    connect(ui->pushButtonCancel,SIGNAL(clicked()),this,SLOT(on_Canceled()));
    connect(ui->pushButtonGenerate,SIGNAL(clicked()),this,SLOT(on_Generate()));
    connect(ui->pushButtonPrevious,SIGNAL(clicked()),this,SLOT(on_PreviousClicked()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(on_TabChanged()));
    ui->pushButtonNext->setEnabled(false);
    ui->tabWidget->setCurrentIndex(0);
}

GridGenerator::~GridGenerator()
{
    delete ui;
}

void GridGenerator::PopulateBlocks()
{

    ui->listWidgetBlocks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllBlockTypes().size(); i++)
    {

        //qDebug() << QString::fromStdString(system.GetAllBlockTypes()[i]);

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem *list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        list_item->setData(1000,QString::fromStdString(system()->GetAllBlockTypes()[i]));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        ui->listWidgetBlocks->addItem(list_item);
    }

}
void GridGenerator::PopulateLinks()
{
    ui->listWidgetLinks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllLinkTypes().size(); i++)
    {

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem* list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        list_item->setData(1000,QString::fromStdString(system()->GetAllLinkTypes()[i]));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        ui->listWidgetLinks->addItem(list_item);
    }
}

System * GridGenerator::system()
{
    if (mainwindow!=nullptr)
        return mainwindow->GetSystem();
    else
        return nullptr;
}

void GridGenerator::on_Selected_block_changed()
{
    ClearPropertiesWindow(objectType::block);
    if (ui->listWidgetBlocks->selectedItems().count()>0)
        Block_type_selected = ui->listWidgetBlocks->selectedItems()[0]->data(1000).toString();
    else
    {
        ui->tab_BlockProperties->setEnabled(false);
        return;

    }

    blockQS = *system()->GetModel(Block_type_selected.toStdString());

    PopulatePropertiesTab(blockQS, ui->gridLayout_Block, objectType::block);

    ui->tab_BlockProperties->setEnabled(true);
    ui->pushButtonNext->setEnabled(true);

}

void GridGenerator::on_Selected_link_changed()
{
    ClearPropertiesWindow(objectType::link);

    if (ui->listWidgetLinks->selectedItems().count()>0)
        Link_type_selected = ui->listWidgetLinks->selectedItems()[0]->data(1000).toString();
    else
    {   ui->tab_LinkProperties->setEnabled(false);
        return;
    }

    linkQS = *system()->GetModel(Link_type_selected.toStdString());

    PopulatePropertiesTab(linkQS, ui->gridLayout_Link, objectType::link);

    ui->tab_LinkProperties->setEnabled(true);
    ui->pushButtonNext->setEnabled(true);
    ui->pushButtonGenerate->setEnabled(true);

}


void GridGenerator::PopulatePropertiesTab(QuanSet& qs, QGridLayout *layout, objectType ObjectType)
{

    for (unsigned int i=0; i<qs.AskableSize(); i++)
    {
        if (ObjectType==objectType::block)
        {
            quan_info qinfo;
            quan_info_Block.insert(qs.GetVarAskable(i)->GetName(),qinfo);
        }
        else if (ObjectType == objectType::link)
        {
            quan_info qinfo;
            quan_info_Link.insert(qs.GetVarAskable(i)->GetName(),qinfo);
        }
        QLabel *label = new QLabel;
        label->setText(QString::fromStdString(qs.GetVarAskable(i)->Description()));
        if (ObjectType == objectType::block)
        {   quan_info_Block[qs.GetVarAskable(i)->GetName()].label = label;
            quan_info_Block[qs.GetVarAskable(i)->GetName()].delegate = qs.GetVarAskable(i)->Delegate();
        }
        else if (ObjectType == objectType::link)
        {   quan_info_Link[qs.GetVarAskable(i)->GetName()].label = label;
            quan_info_Link[qs.GetVarAskable(i)->GetName()].delegate = qs.GetVarAskable(i)->Delegate();
        }
        layout->addWidget(label,i+1,0);
        label->setMaximumSize(max_size_x,max_size_y);
        label->setMinimumSize(min_size_x,min_size_y);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label->setToolTip(QString::fromStdString(qs.GetVarAskable(i)->Description()));
        if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("UnitBox"))
        {
            UnitTextBox3 *editor = new UnitTextBox3(QStyleOptionViewItem() ,this);
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            editor->setUnit(QString::fromStdString(qs.GetVarAskable(i)->Unit()));
            editor->setUnitsList(XString::reform(QString::fromStdString(qs.GetVarAskable(i)->Units()).split(";")));
            editor->setMaximumSize(max_size_x,max_size_y);
            editor->setMinimumSize(min_size_x,min_size_y);
            layout->addWidget(editor,i+1,1);
            UnitTextBox3 *incrementTxtBox_H = new UnitTextBox3(QStyleOptionViewItem(),false ,this);
            UnitTextBox3 *incrementTxtBox_V = new UnitTextBox3(QStyleOptionViewItem(),false ,this);
            incrementTxtBox_H->setUnit(QString::fromStdString(qs.GetVarAskable(i)->Unit()));
            incrementTxtBox_H->setUnitsList(XString::reform(QString::fromStdString(qs.GetVarAskable(i)->Units()).split(";")));
            incrementTxtBox_V->setUnit(QString::fromStdString(qs.GetVarAskable(i)->Unit()));
            incrementTxtBox_V->setUnitsList(XString::reform(QString::fromStdString(qs.GetVarAskable(i)->Units()).split(";")));

            if (ObjectType == objectType::block)
            {   quan_info_Block[qs.GetVarAskable(i)->GetName()].increment_H = incrementTxtBox_H;
                quan_info_Block[qs.GetVarAskable(i)->GetName()].increment_V = incrementTxtBox_V;
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].increment_H = incrementTxtBox_H;
                quan_info_Link[qs.GetVarAskable(i)->GetName()].increment_V = incrementTxtBox_V;
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            incrementTxtBox_H->setMaximumSize(max_size_x/2,max_size_y);
            incrementTxtBox_H->setMinimumSize(min_size_x,min_size_y);
            incrementTxtBox_V->setMaximumSize(max_size_x/2,max_size_y);
            incrementTxtBox_V->setMinimumSize(min_size_x,min_size_y);
            layout->addWidget(incrementTxtBox_H,i+1,2);
            layout->addWidget(incrementTxtBox_V,i+1,3);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ValueBox"))
        {   QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            editor->setMaximumSize(max_size_x,max_size_y);
            editor->setMinimumSize(min_size_x,min_size_y);

            layout->addWidget(editor,i+1,1);
            QLineEdit *incrementTxtBox_H = new QLineEdit();
            QLineEdit *incrementTxtBox_V = new QLineEdit();
            if (ObjectType == objectType::block)
            {   quan_info_Block[qs.GetVarAskable(i)->GetName()].increment_H = incrementTxtBox_H;
                quan_info_Block[qs.GetVarAskable(i)->GetName()].increment_V = incrementTxtBox_V;
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].increment_H = incrementTxtBox_H;
                quan_info_Link[qs.GetVarAskable(i)->GetName()].increment_V = incrementTxtBox_V;
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            layout->addWidget(incrementTxtBox_H,i+1,2);
            layout->addWidget(incrementTxtBox_V,i+1,3);
            if (qs.GetVarAskable(i)->GetName() == "x")
                incrementTxtBox_H->setText("300");
            if (qs.GetVarAskable(i)->GetName() == "y")
                incrementTxtBox_V->setText("300");
            incrementTxtBox_H->setMaximumSize(max_size_x/2,max_size_y);
            incrementTxtBox_H->setMinimumSize(min_size_x,min_size_y);
            incrementTxtBox_V->setMaximumSize(max_size_x/2,max_size_y);
            incrementTxtBox_V->setMinimumSize(min_size_x,min_size_y);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("String"))
        {   QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            editor->setMaximumSize(max_size_x,max_size_y);
            editor->setMinimumSize(min_size_x,min_size_y);
            if (qs.GetVarAskable(i)->GetName()=="name")
            {
                if (ObjectType == objectType::block)
                {
                    editor->setText(Block_type_selected);
                }
                else if (ObjectType == objectType::link)
                {
                    editor->setText(Link_type_selected);
                }
            }
            layout->addWidget(editor,i+1,1);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("MultiComboBox"))
        {
            QListWidget *editor = new QListWidget(this);
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            foreach (QString item , allItems)
            {
                QListWidgetItem* wi = new QListWidgetItem(item);
                //wi.setText(item);
                editor->addItem(wi);
            }
            editor->setSelectionMode(QAbstractItemView::MultiSelection);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ComboBox"))
        {
            QComboBox *editor = new QComboBox(this);
            editor->setFrame(false);
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("CheckBox"))
        {
            QCheckBox *editor = new QCheckBox(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("DirBrowser"))
        {
            QPushButton *editor = new QPushButton(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            QObject::connect(editor, SIGNAL(clicked()), this, SLOT(dirBrowserClicked()));
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Browser"))
        {
            QPushButton *editor = new QPushButton(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            QObject::connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }

        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ListBox"))
        {
            QComboBox *editor = new QComboBox(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            //connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Memo"))
        {
            QTextEdit* editor = new QTextEdit(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            editor->setWordWrapMode(QTextOption::NoWrap);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("expressionEditor"))
        {
            QStringList words;
            for (unsigned int i=0; i<qs.AllConstituents().size(); i++)
                 words << QString::fromStdString(qs.AllConstituents()[i]);

            for (unsigned int i=0; i<qs.AllReactionParameters().size(); i++)
                 words << QString::fromStdString(qs.AllReactionParameters()[i]);

            QString objname;
            if (qs.Count("object")>0)
                objname = QString::fromStdString(qs.GetVar("object").GetProperty());

            Object *obj = nullptr;
            if (objname!="")
                obj = system()->object(objname.toStdString());

            expEditor* editor = new expEditor(obj, words, nullptr, this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }
        else
        {
            QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));

            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                quan_info_Block[qs.GetVarAskable(i)->GetName()].value = editor;
            }
            else if (ObjectType == objectType::link)
            {
                quan_info_Link[qs.GetVarAskable(i)->GetName()].value = editor;
            }
        }

    }

    if (ObjectType == objectType::block)
    {
        verticalSpacer_blocks = new QSpacerItem(20, 40, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
        ui->gridLayout_Block->addItem(verticalSpacer_blocks, qs.AskableSize(), 1, 1, 1);

    }
    else if (ObjectType == objectType::link)
    {
        verticalSpacer_links = new QSpacerItem(20, 40, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
        ui->gridLayout_Link->addItem(verticalSpacer_links, qs.AskableSize(), 1, 1, 1);
    }

}


void GridGenerator::ClearPropertiesWindow(objectType ObjectType)
{
    if (ObjectType == objectType::block)
    {   for (QMap<string,quan_info>::iterator i = quan_info_Block.begin(); i!=quan_info_Block.end(); i++)
        {   ui->gridLayout_Block->removeWidget(i->label);
            delete i->label;
            ui->gridLayout_Block->removeWidget(i->value);
            delete i->value;
            if (i->increment_H!=nullptr)
            {   ui->gridLayout_Block->removeWidget(i->increment_H);
                delete i->increment_H;
            }
            if (i->increment_V!=nullptr)
            {   ui->gridLayout_Block->removeWidget(i->increment_V);
                delete i->increment_V;
            }

        }

        if (verticalSpacer_blocks!=nullptr)
        {   ui->gridLayout_Block->removeItem(verticalSpacer_blocks);
            delete verticalSpacer_blocks;
        }

        quan_info_Block.clear();

    }
    if (ObjectType == objectType::link)
    {   for (QMap<string,quan_info>::iterator i = quan_info_Link.begin(); i!=quan_info_Link.end(); i++)
        {   ui->gridLayout_Link->removeWidget(i->label);
            delete i->label;
            ui->gridLayout_Link->removeWidget(i->value);
            delete i->value;
            if (i->increment_H!=nullptr)
            {   ui->gridLayout_Link->removeWidget(i->increment_H);
                delete i->increment_H;
            }
            if (i->increment_V!=nullptr)
            {   ui->gridLayout_Link->removeWidget(i->increment_V);
                delete i->increment_V;
            }

        }
        if (verticalSpacer_links!=nullptr)
        {   ui->gridLayout_Link->removeItem(verticalSpacer_links);
            delete verticalSpacer_links;
        }

        quan_info_Link.clear();

    }

}

void GridGenerator::browserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(mainwindow,
            tr("Open"), "",
            tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"));
    if (fileName!="")
    {
        if (QFile(fileName).exists())
        {   QObject::sender()->setProperty("text",fileName);
            dynamic_cast<QPushButton*>(sender())->setToolTip(fileName);
        }
        else {
            QMessageBox::information(mainwindow, "File not found!", "File " + fileName + " was not found!", QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        }
    }


}

void GridGenerator::UpdateToolTipes()
{

}

void GridGenerator::on_NextClicked()
{
    if (ui->tabWidget->currentIndex()<ui->tabWidget->count()-1)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);

    on_TabChanged();
}
void GridGenerator::on_PreviousClicked()
{
    if (ui->tabWidget->currentIndex()>0)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()-1);

    on_TabChanged();
}

void GridGenerator::on_TabChanged()
{
    if (ui->tabWidget->currentIndex()==0)
        ui->pushButtonPrevious->setEnabled(false);
    else
        ui->pushButtonPrevious->setEnabled(true);
    if (ui->tabWidget->currentIndex()==ui->tabWidget->count()-1)
        ui->pushButtonNext->setEnabled(false);
    else
        ui->pushButtonNext->setEnabled(true);

}

void GridGenerator::on_Canceled()
{
    this->close();
}
void GridGenerator::on_Generate()
{
    mainwindow->undoData.SetActiveSystem(system());
    GenerateBlocks();
    GenerateLinks();

    mainwindow->RefreshTreeView();
    mainwindow->GetDiagramView()->repaint();
    mainwindow->undoData.AppendAfterActive(system());
    mainwindow->RecreateGraphicItemsFromSystem();
    this->close();
}

bool GridGenerator::GenerateBlocks()
{
    for (int i=0; i<ui->spinBox_nx->text().toInt(); i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt(); j++)
        {
            string name = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")").toStdString();
            if (system()->object(name)!=nullptr)
            {
                QMessageBox::question(this, QString("An object called " + QString::fromStdString(name) + "already exists. Considering chamaging the name of the object."), QString::fromStdString(system()->lasterror()), QMessageBox::Ok);
                return false;
            }
        }
    for (int i=0; i<ui->spinBox_nx->text().toInt(); i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt(); j++)
        {
            Block block;
            if (!block.SetQuantities(system()->GetMetaModel(),Block_type_selected.toStdString()))
            {
                mainwindow->LogError(QString::fromStdString(block.lasterror()));
                return false;
            }


            block.SetType(Block_type_selected.toStdString());

            string name = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")").toStdString();
            block.SetName(name);
            //qDebug() << "Adding Block to the system";

            system()->AddBlock(block);
            system()->object(name)->SetName(name);
            system()->AddAllConstituentRelateProperties(system()->block(name));
            //Node *node = new Node(mainwindow->GetDiagramView(),system());
            for (QMap<string,quan_info>::iterator it = quan_info_Block.begin(); it!=quan_info_Block.end(); it++)
                AssignProperty(name,blockQS,it,i,j);

            //qDebug() << "DiagramView Repainted!";
            system()->object(name)->AssignRandomPrimaryKey();
            system()->SetVariableParents();
            //node->SetObject(system()->object(name));

            mainwindow->LogAddDelete("Block '" + QString::fromStdString(name) + "' was added!");


        }
    return true;
}
bool GridGenerator::GenerateLinks()
{
    for (int i=0; i<ui->spinBox_nx->text().toInt(); i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt()-1; j++)
        {
            string name = (dynamic_cast<QLineEdit*>(quan_info_Link["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")-("+QString::number(i+1)+":"+QString::number(j+2)+")").toStdString();
            if (system()->object(name)!=nullptr)
            {
                QMessageBox::question(this, QString("An object called " + QString::fromStdString(name) + "already exists. Considering chamaging the name of the object."), QString::fromStdString(system()->lasterror()), QMessageBox::Ok);
                return false;
            }
        }

    for (int i=0; i<ui->spinBox_nx->text().toInt()-1; i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt(); j++)
        {
            string name = (dynamic_cast<QLineEdit*>(quan_info_Link["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")-("+QString::number(i+2)+":"+QString::number(j+1)+")").toStdString();
            if (system()->object(name)!=nullptr)
            {
                QMessageBox::question(this, QString("An object called " + QString::fromStdString(name) + "already exists. Considering chamaging the name of the object."), QString::fromStdString(system()->lasterror()), QMessageBox::Ok);
                return false;
            }
        }

    for (int i=0; i<ui->spinBox_nx->text().toInt(); i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt()-1; j++)
        {
            Link link;
            if (!link.SetQuantities(system()->GetMetaModel(),Link_type_selected.toStdString()))
            {
                mainwindow->LogError(QString::fromStdString(link.lasterror()));
                return false;
            }

            link.SetType(Block_type_selected.toStdString());

            string name = (dynamic_cast<QLineEdit*>(quan_info_Link["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")-("+QString::number(i+1)+":"+QString::number(j+2)+")").toStdString();
            string srcname = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")").toStdString();
            string destname = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+2)+")").toStdString();
            link.SetName(name);
            //qDebug() << "Adding Block to the system";

            system()->AddLink(link,srcname,destname);
            system()->object(name)->SetName(name);
            system()->AddAllConstituentRelateProperties(system()->link(name));
            for (QMap<string,quan_info>::iterator it = quan_info_Link.begin(); it!=quan_info_Link.end(); it++)
                AssignProperty(name,linkQS,it,i,j);
            //qDebug() << "DiagramView Repainted!";
            system()->object(name)->AssignRandomPrimaryKey();
            system()->SetVariableParents();
            //node->SetObject(system()->object(name));

            mainwindow->LogAddDelete("Link '" + QString::fromStdString(name) + "' was added!");


        }

    for (int i=0; i<ui->spinBox_nx->text().toInt()-1; i++)
        for (int j=0; j<ui->spinBox_ny->text().toInt(); j++)
        {
            Link link;
            if (!link.SetQuantities(system()->GetMetaModel(),Link_type_selected.toStdString()))
            {
                mainwindow->LogError(QString::fromStdString(link.lasterror()));
                return false;
            }

            link.SetType(Block_type_selected.toStdString());

            string name = (dynamic_cast<QLineEdit*>(quan_info_Link["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")-("+QString::number(i+2)+":"+QString::number(j+1)+")").toStdString();
            string srcname = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+1)+":"+QString::number(j+1)+")").toStdString();
            string destname = (dynamic_cast<QLineEdit*>(quan_info_Block["name"].value)->text()+ "("+QString::number(i+2)+":"+QString::number(j+1)+")").toStdString();
            link.SetName(name);
            //qDebug() << "Adding Block to the system";

            system()->AddLink(link,srcname,destname);
            system()->object(name)->SetName(name);
            system()->AddAllConstituentRelateProperties(system()->link(name));
            for (QMap<string,quan_info>::iterator it = quan_info_Link.begin(); it!=quan_info_Link.end(); it++)
                AssignProperty(name,linkQS,it,i,j);
            //qDebug() << "DiagramView Repainted!";
            system()->object(name)->AssignRandomPrimaryKey();
            system()->SetVariableParents();
            //node->SetObject(system()->object(name));

            mainwindow->LogAddDelete("Link '" + QString::fromStdString(name) + "' was added!");


        }
    return true;

}

bool GridGenerator::AssignProperty(const string &name, QuanSet &quanset, QMap<string,quan_info>::iterator it, int i, int j)
{
    bool r;
    if (QString::fromStdString(it->delegate).contains("UnitBox"))
    {
        double value=0;
        double increment_x=0;
        double increment_y=0;
        double coefficient = XString::coefficient(QString::fromStdString(quanset.GetVar(it.key()).Unit()));

        value = dynamic_cast<UnitTextBox3*>(it->value)->text().toDouble()*coefficient;
        increment_x = dynamic_cast<UnitTextBox3*>(it->increment_H)->text().toDouble();
        increment_y = dynamic_cast<UnitTextBox3*>(it->increment_V)->text().toDouble();
        r = system()->object(name)->Variable(it.key())->SetVal(value+i*increment_x+j*increment_y,Expression::timing::both);
    }
    else if (QString::fromStdString(it->delegate).contains("ValueBox"))
    {
        double value=0;
        double increment_x=0;
        double increment_y=0;
        value = dynamic_cast<QLineEdit*>(it->value)->text().toDouble();
        increment_x = dynamic_cast<QLineEdit*>(it->increment_H)->text().toDouble();
        increment_y = dynamic_cast<QLineEdit*>(it->increment_V)->text().toDouble();
        r = system()->object(name)->Variable(it.key())->SetVal(value+i*increment_x+j*increment_y,Expression::timing::both);
    }
    else if (QString::fromStdString(it->delegate).contains("String") && it.key()!="name")
    {
        string value;
        value = dynamic_cast<QLineEdit*>(it->value)->text().toStdString();
        r = system()->object(name)->Variable(it.key())->SetProperty(value,true);
    }
    else if (QString::fromStdString(it->delegate).contains("ComboBox"))
    {
        string value = dynamic_cast<QComboBox*>(it->value)->currentText().toStdString();
        r = system()->object(name)->Variable(it.key())->SetProperty(value,true);
    }
    else if (QString::fromStdString(it->delegate).contains("MultiComboBox"))
    {
        QListWidget *list = dynamic_cast<QListWidget*>(it->value);
        QStringList selectedList;
        //list->selectAll();
        for (int i = 0; i < list->count(); i++)
            if (list->item(i)->isSelected() && list->item(i)->text().size())
                selectedList.append(list->item(i)->text());
        string value = selectedList.join(':').toStdString();
        r = system()->object(name)->Variable(it.key())->SetProperty(value,true);

    }
    else if (QString::fromStdString(it->delegate).contains("Browser"))
    {
        string value = dynamic_cast<QPushButton*>(it->value)->text().toStdString();
        r = system()->object(name)->Variable(it.key())->SetProperty(value,true);
    }
    else if (QString::fromStdString(it->delegate).contains("expressionEditor"))
    {
        string value = dynamic_cast<expEditor*>(it->value)->text().toStdString();
        r = system()->object(name)->Variable(it.key())->SetProperty(value,false);
    }
    return r;
}
