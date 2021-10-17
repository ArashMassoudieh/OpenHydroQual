#ifndef GRIDGENERATOR_H
#define GRIDGENERATOR_H

#include <QDialog>
#include "System.h"

constexpr int max_size_x = 300;
constexpr int max_size_y = 300;


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
    QString Block_type_selected;
    QString Link_type_selected;
    QList<QLabel*> BlockPropertiesLabels;
    QList<QLabel*> LinkPropertiesLabels;
    QList<QWidget*> BlockPropertiesValues;
    QList<QWidget*> LinkPropertiesValues;
    QList<QWidget*> BlockPropertiesIncrements;
    QList<QWidget*> LinkPropertiesIncrements;
    void ClearPropertiesWindow();
    void UpdateToolTipes();
private slots:
    void on_Selected_item_changed();
    void browserClicked();
};

#endif // GRIDGENERATOR_H
