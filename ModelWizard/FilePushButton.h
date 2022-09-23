#pragma once

#include <QObject>
#include <qpushbutton.h>

class FilePushButton : public QPushButton
{
	Q_OBJECT
public:
	FilePushButton(QWidget *parent);
	~FilePushButton();
public slots:
	void browserClicked();
};
