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
//    getFrame();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getFrame()
{
    camera->startTriggerMode();
    //camera->start();


    memcpy(buffer[indexBuffer + 1], (uchar*)camera->globalImageBuffer, WIDTH * HEIGHT);
    findHoles();

    if(indexBuffer + 1 >= 9)
        indexBuffer = 0;
    else
        indexBuffer++;


    getFrame();//this->thread()->sleep(1000);
}

void MainWindow::findHoles()
{
    cv::Mat img(600,800,CV_8U,buffer[indexBuffer + 1]);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    int lowerd = 200, upperb = 256;
    cv::inRange(img, (lowerd,lowerd,lowerd), (upperb ,upperb, upperb),img);
    Mat kernel = cv::Mat::ones(15,15,CV_8U);
    morphologyEx(img,img, cv::MORPH_OPEN,kernel);
//    Mat contours;
    findContours(img,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE,Point(0,0));
    Mat drawing = Mat::zeros(img.size(),CV_8U );
    RNG rng(12345);

    vector<Point2f> center(contours.size());
    vector<float> radius (contours.size());

    for(int i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255) , rng.uniform(0, 255));
        drawContours(drawing, contours , i , color , 2,8,hierarchy,0,Point());

        minEnclosingCircle(Mat(contours[i]),center[i],radius[i]);

        qDebug()<<"x: "<<center[i].x<<" y: "<<center[i].y<<" radius : "<<radius[i];
    }

//    namedWindow("test");
//    imshow("test",drawing);

    memcpy(buffer[indexBuffer + 1], drawing.data,WIDTH * HEIGHT);
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
