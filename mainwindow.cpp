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
//    isHollFinding = 1;
    ShowOriginalImage = 0;
    showFullSizeImage = 0;
    objectThr = 220;
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

    for(int i = 0; i < 10; i++)
        bufferCircle[i] = (char *)malloc(WIDTH * HEIGHT);

    for(int i = 0; i < 10; i++)
        circleSize[i] = 0;

    tmrGraphicView.setInterval(500);
    connect(&tmrGraphicView,SIGNAL(timeout()),SLOT(updateGraphicView()));
    tmrGraphicView.start();

    dialogConfig = new Dialog(0,this);
    QtConcurrent::run(this,&MainWindow::getFrameWhile);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getFrameWhile()
{
    while(1)
        getFrame();
}

void MainWindow::getFrame()
{
    if(isTriggeMode)
        camera->startTriggerMode();
    else
        camera->start();
//    return;
    startCapture = 1;

    int index = indexBuffer;
    memcpy(buffer[index], (uchar*)camera->globalImageBuffer, WIDTH * HEIGHT);

    if(isTriggeMode)
    {
        // int index = (indexBuffer == 0) ? 9 : indexBuffer - 1;
        char test[800*600];
        memcpy(test,buffer[index],800*600);
        findDiameter(test,index);
        findHoles(index);

    }

    indexBuffer = (index + 1) % 10;

//    if(!isTriggeMode)
//    {
//        QEventLoop loop;
//        QTimer::singleShot(500,&loop,SLOT(quit()));
//        loop.exec();
//    }

}

void MainWindow::findHoles(int index)
{
    if(showFullSizeImage)
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
            memcpy(buffer[index], drawing.data,WIDTH * HEIGHT);
    }
    else
    {
        char *tmpBuffer = (char *)malloc(circleSize[index] * circleSize[index]);
        memcpy(tmpBuffer,bufferCircle[index],circleSize[index] * circleSize[index]);
        cv::Mat img(circleSize[index],circleSize[index],CV_8U,tmpBuffer);
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
            memcpy(bufferCircle[index], drawing.data,circleSize[index] * circleSize[index]);
    }
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

    if(showFullSizeImage || (!isTriggeMode))
        imgUpdateView = new QImage((uchar*)buffer[index], WIDTH, HEIGHT, QImage::Format_Indexed8);
    else
        imgUpdateView = new QImage((uchar*)bufferCircle[index], circleSize[index], circleSize[index], QImage::Format_Indexed8);

    scene->addPixmap(QPixmap::fromImage(*imgUpdateView));
    ui->graphicsView->show();
}

void MainWindow::on_pushButton_clicked()
{
    //    Dialog dialog(0,this);
    dialogConfig->show();
}

void MainWindow::findDiameter(char *input,int index)
{
    int Ui,Uj,Ri,Rj;
    for(int j = 10;j < 600; j++)
        for(int i = 799; i > 300; i--)
        {
            int k = (uchar)input[i + 800 * j];
            if(k < objectThr)
            {
                // break
                Ui = i;
                Uj = j;
                i = 0;
                j = 600;
            }
        }


    for(int i = 799; i > 300; i--)
        for(int j = 10;j < 400; j++)
        {
            int k = (uchar)input[i + 800 * j];
            if(k < objectThr)
            {
                Ri = i;
                Rj = j;
                i = 0;
                j = 600;
            }
        }

    int k = ((0.353553 * ( Rj - Uj) * 2 - 5));
    if(k % 2 != 0)
        k --;
    //    qDebug()<<"k = "<<k;
    circleSize[index] = k*2;


    int I ,J;
    for(int j = Rj-k; j < Rj+k; j++)
        for(int i = Ui-k; i < Ui+k; i++)
        {
            I = (i - Ui+k); J = (j - Rj+k);
            bufferCircle[index][I + J * k*2] = input[i + j * 800];
        }

}
