#ifndef GRIDGENERATOR_H
#define GRIDGENERATOR_H

#include <QDialog>
#include "System.h"
#include <QMap>

constexpr int max_size_x = 300;
constexpr int max_size_y = 300;
constexpr int min_size_x = 100;
constexpr int min_size_y = 30;

enum class objectType {block, link_x, link_y};

struct quan_info
{
    QLabel *label=nullptr;
    QWidget *value=nullptr;
    QWidget *increment_H = nullptr;
    QWidget *increment_V = nullptr;
    string delegate = "";
};

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
    QMap<string, quan_info> quan_info_Block;
    QMap<string, quan_info> quan_info_Link_x;
    QMap<string, quan_info> quan_info_Link_y;
    void ClearPropertiesWindow(objectType);
    void UpdateToolTipes();
    void PopulatePropertiesTab(QuanSet&, QGridLayout *,objectType);
    QSpacerItem *verticalSpacer_links_x = nullptr;
    QSpacerItem *verticalSpacer_links_y = nullptr;
    QSpacerItem *verticalSpacer_blocks = nullptr;
    QuanSet blockQS;
    QuanSet linkQS;
    bool GenerateBlocks();
    bool GenerateLinks();
    void connectLinkTextBoxes();
    bool AssignProperty(const string &name, QuanSet &quanset, QMap<string,quan_info>::iterator it, int i, int j);
private slots:
    void on_Selected_block_changed();
    void on_Selected_link_changed();
    void browserClicked();
    void on_NextClicked();
    void on_PreviousClicked();
    void on_TabChanged();
    void on_Canceled();
    void on_Generate();
};

#endif // GRIDGENERATOR_H
