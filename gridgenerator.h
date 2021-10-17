#ifndef GRIDGENERATOR_H
#define GRIDGENERATOR_H

#include <QDialog>
#include "System.h"

constexpr int max_size_x = 300;
constexpr int max_size_y = 300;
constexpr int min_size_x = 100;
constexpr int min_size_y = 30;

enum class objectType {block, link};

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
    void ClearPropertiesWindow(objectType);
    void UpdateToolTipes();
    void PopulatePropertiesTab(QuanSet&, QGridLayout *,objectType);
    QSpacerItem *verticalSpacer_links = nullptr;
    QSpacerItem *verticalSpacer_blocks = nullptr;

private slots:
    void on_Selected_block_changed();
    void on_Selected_link_changed();
    void browserClicked();
    void on_NextClicked();
    void on_PreviousClicked();
    void on_TabChanged();
};

#endif // GRIDGENERATOR_H
