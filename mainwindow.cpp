#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    indexBuffer = -1;

    //camera
    camera = new basler();
    if(camera->connect())
        qDebug()<<"camera is connected";
    else
        qDebug()<<"cannot connect to camera !!!";

    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);

    for(int i = 0; i < 10; i++)
        buffer[i] = (char *)malloc(WIDTH * HEIGHT);

    //QtConcurrent::run(MainWindow::getFrame);
   // QtConcurrent::run(this,&MainWindow::getFrame);

    tmrGraphicView.setInterval(500);
    connect(&tmrGraphicView,SIGNAL(timeout()),SLOT(updateGraphicView()));
    tmrGraphicView.start();

    QtConcurrent::run(this,&MainWindow::getFrame);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getFrame()
{
//    camera->startTriggerMode();
    camera->start();


    memcpy(buffer[indexBuffer + 1], (uchar*)camera->globalImageBuffer, WIDTH * HEIGHT);

    if(indexBuffer + 1 >= 9)
        indexBuffer = 0;
    else
        indexBuffer++;


    getFrame();
}

void MainWindow::updateGraphicView()
{
//    getFrame();
    if(indexBuffer < 0)
        return;
    static int lastBufferIndex = -1;
    if(lastBufferIndex == indexBuffer)
        return;

    lastBufferIndex = indexBuffer;
    ui->graphicsView->scene()->clear();

    imgUpdateView = new QImage((uchar*)buffer[indexBuffer], WIDTH, HEIGHT, QImage::Format_Indexed8);

    scene->addPixmap(QPixmap::fromImage(*imgUpdateView));
    ui->graphicsView->show();
}
