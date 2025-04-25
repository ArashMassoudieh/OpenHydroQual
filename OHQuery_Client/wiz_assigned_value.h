#pragma once
#include <qstring.h>
class wiz_assigned_value
{
public:
	wiz_assigned_value();
	wiz_assigned_value(QString s);
	wiz_assigned_value(const wiz_assigned_value &s);
	QString toQString();
	~wiz_assigned_value();
	QString value_param_expression; 
	QString unit; 
	QString entity; 
	QString parameter_name; 
	QString value; 
	QString _last_error;
};

QString extract_between(QString str, QString del1, QString del2);
QString extract_between(QString str, char del1, char del2);
