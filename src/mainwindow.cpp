#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

        // create our object and add it to the scene
    item = new GItem();
    item->Properties.ImageFileName = QString("C:\\Projects\\GIFMod\\src\\resources\\Icons\\Darcy.png");
    item->Properties.Name = "myTestBlock";
    item->Properties.size_h = 100;
    scene->addItem(item);
}

MainWindow::~MainWindow()
{
    delete ui;
}
