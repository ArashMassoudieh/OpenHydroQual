#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtAquifolium.h"

class QtAquifolium : public QMainWindow
{
	Q_OBJECT

public:
	QtAquifolium(QWidget *parent = Q_NULLPTR);

private:
	Ui::QtAquifoliumClass ui;
};
