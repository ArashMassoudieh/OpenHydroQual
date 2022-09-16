#pragma once
#include <qstring.h>
#include "MajorBlock.h" 
#include <QMap>

class WizardScript
{
public: 
	WizardScript(); 
	WizardScript(const QString& filename);
	WizardScript(const WizardScript &WS);
	WizardScript& operator=(const WizardScript& WS);
	QString Icon(); 
	QString Name();
	QString Description(); 
private:
	QMap<QString, MajorBlock> MajorBlocks;

};

