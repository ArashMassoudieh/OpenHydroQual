#ifndef WIZARD_SELECT_DIALOG_H
#define WIZARD_SELECT_DIALOG_H

#include <QWidget>
#include <qdialog.h>
#include "ui_wizard_select_dialog.h"
#include "qstringlist.h"
#include "qmap.h"

struct plugin_information
{
    QString Filename;
    QString Description;
    QString IconFileName;
};

class MainWindow;

class Wizard_select_dialog : public QDialog
{
	Q_OBJECT

public:
    Wizard_select_dialog(MainWindow *parent = 0);
	~Wizard_select_dialog();

private:
	Ui::Wizard_select_dialog ui;
    QString selected_template;
    QVector<plugin_information> DefaultPlugins;
    bool get_templates(const QString &TemplateList);


private slots:
	void on_ok_clicked();
	void on_cancel_clicked();


};


#endif // WIZARD_SELECT_DIALOG_H
