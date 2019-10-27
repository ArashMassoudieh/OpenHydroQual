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
    item->ImageFileName = "/home/arash/Projects/Aquifolium_GUI/resources/Icons/Darcy.png";
    item->Name = "myTestBlock";
    item->size_h = 30;
    scene->addItem(item);
}

MainWindow::~MainWindow()
{
    delete ui;
}
