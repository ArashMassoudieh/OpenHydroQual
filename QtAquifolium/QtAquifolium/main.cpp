#include "QtAquifolium.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtAquifolium w;
	w.show();
	return a.exec();
}
