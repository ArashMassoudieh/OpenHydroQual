#ifndef WIZARD_SELECT_DIALOG_H
#define WIZARD_SELECT_DIALOG_H

#include <QWidget>
#include <qdialog.h>
#include "ui_wizard_select_dialog.h"
#include "qstringlist.h"
#include "qmap.h"
#include "mProplist.h"

class MainWindow;

class Wizard_select_dialog : public QDialog
{
	Q_OBJECT

public:
	Wizard_select_dialog(QString *template_selected, MainWindow *parent = 0);
	~Wizard_select_dialog();
	QMap<QString, QString> template_filenames;
private:
	Ui::Wizard_select_dialog ui;
	QString *selected_template; 

private slots:
	void on_ok_clicked();
	void on_cancel_clicked();
};

QMap<QString, QString> get_templates(QString &path); 
QMap<QString, QString> get_icons(QString & path);
QMap<QString, QString> get_descriptions(QString & path); 

#endif // WIZARD_SELECT_DIALOG_H
