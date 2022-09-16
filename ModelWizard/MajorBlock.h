#pragma once
#include <qstring.h>


class MajorBlock
{
public:
	MajorBlock();
	MajorBlock(const QString& filename);
	MajorBlock(const MajorBlock& WS);
	MajorBlock& operator=(const MajorBlock& WS);
	QString Name();
	QString Description();
private:

};
