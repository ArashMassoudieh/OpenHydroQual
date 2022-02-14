#include "wizard_select_dialog.h"
#include "mainwindow.h"
#include "utilityfuncs.h"

Wizard_select_dialog::Wizard_select_dialog(MainWindow *_parent)
    : QDialog(_parent)
{
    parent = _parent;
    ui.setupUi(this);
	ui.listWidget->clear(); 

    QString path = QString::fromStdString(RESOURCE_DIRECTORY+"/");
    ui.listWidget->setIconSize(QSize(100, 100));
	
    if (!get_templates(path + "default_templates.list"))
         this->close();


    for (int i=0; i<DefaultPlugins.size(); i++ )
	{
        QIcon icon(path + "Icons/" + DefaultPlugins[i].IconFileName);
        QListWidgetItem *list_item = new QListWidgetItem(icon, DefaultPlugins[i].Description);
        list_item->setToolTip(DefaultPlugins[i].Description);
		
		ui.listWidget->addItem(list_item);
	}

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(on_ok_clicked()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(on_cancel_clicked()));

	
}

Wizard_select_dialog::~Wizard_select_dialog()
{
	
}

bool Wizard_select_dialog::get_templates(const QString & TemplateListFileName)
{
    QFile inputFile(TemplateListFileName);
    if (!inputFile.open(QIODevice::ReadOnly))
        return false;

    QString path = QString::fromStdString(RESOURCE_DIRECTORY+"/");

    QTextStream line(&inputFile);
    while (!line.atEnd())
    {
        QString content = line.readLine();
        QStringList contentlist = content.split(",");
        plugin_information plugin_info;
        if (contentlist.size()>=3 && fileExists(path+contentlist[1].trimmed()))
        {
            plugin_info.Filename = path + contentlist[1].trimmed();
            plugin_info.Description = contentlist[0].trimmed();
            plugin_info.IconFileName = contentlist[2].trimmed();
        }
        DefaultPlugins.append(plugin_info);
    }

    return true;
}



void Wizard_select_dialog::on_ok_clicked()
{
    selected_template = DefaultPlugins[ui.listWidget->currentRow()].Filename;
    if (parent)
    {
        parent->addplugin(DefaultPlugins[ui.listWidget->currentRow()].Filename);
    }
    this->close();
}

void Wizard_select_dialog::on_cancel_clicked()
{
    selected_template = "";
	this->close();
}
