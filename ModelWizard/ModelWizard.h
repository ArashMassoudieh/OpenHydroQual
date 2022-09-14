#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ModelWizard.h"

class ModelWizard : public QMainWindow
{
    Q_OBJECT

public:
    ModelWizard(QWidget *parent = Q_NULLPTR);

private:
    Ui::ModelWizardClass ui;
};
