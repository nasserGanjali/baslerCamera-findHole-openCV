#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    indexBuffer = 0;
    startCapture = 0;
    isTriggeMode = 1;
    isHollFinding = 1;
    ShowOriginalImage = 0;
    CV_lowerd = 200; CV_upperb = 255;
    CV_kernelGain = 15;

    //camera
    camera = new basler(this);
    if(camera->connect())
        qDebug()<<"camera is connected";
    else
        qDebug()<<"cannot connect to camera !!!";

    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);

    for(int i = 0; i < 10; i++)
        buffer[i] = (char *)malloc(WIDTH * HEIGHT);

    tmrGraphicView.setInterval(500);
    connect(&tmrGraphicView,SIGNAL(timeout()),SLOT(updateGraphicView()));
    tmrGraphicView.start();

    dialogConfig = new Dialog(0,this);
    QtConcurrent::run(this,&MainWindow::getFrame);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getFrame()
{
    if(isTriggeMode)
        camera->startTriggerMode();
    else
        camera->start();

    startCapture = 1;

    memcpy(buffer[indexBuffer], (uchar*)camera->globalImageBuffer, WIDTH * HEIGHT);

    if(isHollFinding)
        findHoles();


    indexBuffer = (indexBuffer + 1) % 10;

    getFrame();//this->thread()->sleep(1000);
}

void MainWindow::findHoles()
{
    char tmpBuffer[WIDTH*HEIGHT];
    memcpy(tmpBuffer,buffer[indexBuffer],WIDTH*HEIGHT);
    cv::Mat img(HEIGHT,WIDTH,CV_8U,tmpBuffer);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //
    cv::inRange(img, (CV_lowerd,CV_lowerd,CV_lowerd), (CV_upperb ,CV_upperb, CV_upperb),img);
    Mat kernel = cv::Mat::ones(CV_kernelGain,CV_kernelGain,CV_8U);
    morphologyEx(img,img, cv::MORPH_OPEN,kernel);

    findContours(img,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE,Point(0,0));

    Mat drawing = Mat::zeros(img.size(),CV_8U );
    RNG rng(12345);

    vector<Point2f> center(contours.size());
    vector<float> radius (contours.size());

    for(int i = 0; i < contours.size(); i++)
    {

        if(!ShowOriginalImage)
        {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255) , rng.uniform(0, 255));
            drawContours(drawing, contours , i , color , 2,8,hierarchy,0,Point());
        }
        minEnclosingCircle(Mat(contours[i]),center[i],radius[i]);

        qDebug()<<"x: "<<center[i].x<<" y: "<<center[i].y<<" radius : "<<radius[i];
    }


    if(!ShowOriginalImage)
        memcpy(buffer[indexBuffer], drawing.data,WIDTH * HEIGHT);
}

void MainWindow::updateGraphicView()
{
    //    getFrame();
    if(!startCapture)
        return;
    int index = indexBuffer - 1;
    if(index < 0)
        index = 9;
    //qDebug()<<index;
    static int lastBufferIndex = -1;
    if(lastBufferIndex == indexBuffer - 1)
        return;

    lastBufferIndex = indexBuffer - 1;
    ui->graphicsView->scene()->clear();

    imgUpdateView = new QImage((uchar*)buffer[index], WIDTH, HEIGHT, QImage::Format_Indexed8);

    scene->addPixmap(QPixmap::fromImage(*imgUpdateView));
    ui->graphicsView->show();
}

void MainWindow::on_pushButton_clicked()
{
    //    Dialog dialog(0,this);
    dialogConfig->show();
}
