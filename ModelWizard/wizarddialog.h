#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <QDialog>
#include "Wizard_Script.h"
#include <QScrollArea>
#include <QTabWidget>
#include <QFormLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include "svgviewer.h"

struct tab {
    QScrollArea *scrollArea;
    QFormLayout *formlayout;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QWidget *scrollAreaWidgetContents;
    QFormLayout *formLayout;
    WizardParameterGroup *parametergroup;
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
    void GenerateModel();
    bool Verify();
    void resizeEvent(QResizeEvent *r=nullptr);

private:
    Ui::WizardDialog *ui;
    QMap<QString,tab> tabs;
    WizardScript SelectedWizardScript;
    QPixmap* diagram_pix = nullptr;
    QGraphicsSvgItem *svgitem = nullptr;
    int currenttabindex=0;
    SVGViewer *svgviewer = nullptr;

public slots:
    void on_next_clicked();
    void on_previous_clicked();
    void on_TabChanged();
    void open_html();

};

#endif // WIZARDDIALOG_H
