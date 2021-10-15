#ifndef GRIDGENERATOR_H
#define GRIDGENERATOR_H

#include <QDialog>
#include "System.h"

class MainWindow;

namespace Ui {
class GridGenerator;
}

class GridGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit GridGenerator(MainWindow *parent = nullptr);
    ~GridGenerator();
    void PopulateBlocks();
    void PopulateLinks();
    System * system();
private:
    Ui::GridGenerator *ui;
    MainWindow *mainwindow;
};

#endif // GRIDGENERATOR_H
