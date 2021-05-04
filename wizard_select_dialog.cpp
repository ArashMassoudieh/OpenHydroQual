#include "wizard_select_dialog.h"
#include "qdir.h"
#include "Wizard/Wizard_Script_Reader.h"
#include "mainwindow.h"

Wizard_select_dialog::Wizard_select_dialog(QString *template_selected, MainWindow *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.listWidget->clear(); 
	ui.listWidget->setIconSize(QSize(200, 200));
	
	QString path = QApplication::applicationDirPath() + "/templates";
	QMap<QString, QString> template_list = get_icons(path);
	QMap<QString, QString> descriptions = get_descriptions(path);
	template_filenames = get_templates(path);

	for (QString name : template_list.keys())
	{
		QIcon icon(path + "/" + template_list[name]);
		QListWidgetItem *list_item = new QListWidgetItem(icon, name);
		list_item->setToolTip(descriptions[name]);
		
		ui.listWidget->addItem(list_item);
	}
	selected_template = template_selected;

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(on_ok_clicked()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(on_cancel_clicked()));

	
}

Wizard_select_dialog::~Wizard_select_dialog()
{
	
}

QMap<QString, QString> get_templates(QString & path)
{
	QDir dir(path);
	QMap<QString, QString> out; 
	foreach(QFileInfo item, dir.entryInfoList())
	{
		if (item.isFile() && item.fileName().right(3).toLower() == "wiz")
		{
			Wizard_Script_Reader W(item.absoluteFilePath());
			out[W.get_script_name()] = item.absoluteFilePath();
		}
	}
	return out; 
}

QMap<QString, QString> get_icons(QString & path)
{
	QDir dir(path);
	QMap<QString, QString> out;
	foreach(QFileInfo item, dir.entryInfoList())
	{
		if (item.isFile() && item.fileName().right(3).toLower() == "wiz")
		{
			Wizard_Script_Reader W(item.absoluteFilePath());
			out[W.get_script_name()] = W.get_script_icon();
		}
	}
	return out;
}

QMap<QString, QString> get_descriptions(QString & path)
{
	QDir dir(path);
	QMap<QString, QString> out;
	foreach(QFileInfo item, dir.entryInfoList())
	{
		if (item.isFile() && item.fileName().right(3).toLower() == "wiz")
		{
			Wizard_Script_Reader W(item.absoluteFilePath());
			out[W.get_script_name()] = W.get_description(); 
		}
	}
	return out;
}

void Wizard_select_dialog::on_ok_clicked()
{
	selected_template->clear(); 
	if (ui.listWidget->selectedItems().size())
	{
		selected_template->append(template_filenames[ui.listWidget->selectedItems()[0]->text()]);
		this->close();
	}
}

void Wizard_select_dialog::on_cancel_clicked()
{
	selected_template->clear();
	this->close();
}
