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
    appClosed = 0;
    //    isHollFinding = 1;
    ShowOriginalImage = 0;
    showFullSizeImage = 0;
    maxHollSize = 40;
    minHollSize = 17;
    defectProdocts = 0;
    totalProdocts = 0;
    historyDefectProdocts = 0;
    historyTotalProdocts = 0;
    imgUpdateView = NULL;
    objectThr = 220;
    CV_lowerd = 200; CV_upperb = 255;
    CV_kernelGain = 15;

    dialogCheckCamera = new dialogTestCamera(0,this,camera);

    //camera
    camera = new basler(this);
    camera->loadConfig();

    if(!isTriggeMode)
    {
        if(camera->connect())
            qDebug()<<"camera is connected";
        else
            qDebug()<<"cannot connect to camera !!!";
    }else
    {
        camera->initTrigger();
    }

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

    GPIO_start();

    loadHistory();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::getFrameWhile()
{
    while(!appClosed)
    {
        //        qDebug()<<"time :"<<QTime::currentTime().toString();
        getFrame();
    }
}

void MainWindow::on_btnTestCamera_clicked()
{
    dialogCheckCamera->show();
}

void MainWindow::getFrame()
{
    bool mode = isTriggeMode;
    int res = 0;
    if(mode)
    {
        if(!camera->Camera->IsOpen())
        {
            camera->closeAllCameras();
            camera->initTrigger();
        }
        triggerTimeout = 0;
        res = camera->startTriggerMode();
        if(res == 0 || res == 1)
            triggerTimeout = res;
        else if(res == -2)
            singleShot();
    }
    else
    {
        if(camera->camera == NULL)
        {
            camera->closeAllCameras();
            if(camera->connect())
                qDebug()<<"camera is connected";
            else
                qDebug()<<"cannot connect to camera !!!";
        }

        camera->start();
    }

    startCapture = 1;

    int index = indexBuffer;
    memcpy(buffer[index], (uchar*)camera->globalImageBuffer, WIDTH * HEIGHT);

    if(mode && !triggerTimeout)
    {
        // int index = (indexBuffer == 0) ? 9 : indexBuffer - 1;
        QtConcurrent::run(this,&MainWindow::Algorithm,index);
    }
    else
        indexBuffer = (index + 1) % 10;

    //    if(!isTriggeMode)
    //    {
    //        QEventLoop loop;
    //        QTimer::singleShot(500,&loop,SLOT(quit()));
    //        loop.exec();
    //    }

}

void MainWindow::Algorithm(int index)
{
    char test[800*600];
    memcpy(test,buffer[index],800*600);
    if(findDiameter(test,index))
        findHoles(index);
    indexBuffer = (index + 1) % 10;
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

        int i = 0;
        int defect = false;
        for(i = 0; i < contours.size(); i++)
        {

            if(!ShowOriginalImage)
            {
                Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255) , rng.uniform(0, 255));
                drawContours(drawing, contours , i , color , 2,8,hierarchy,0,Point());
            }
            minEnclosingCircle(Mat(contours[i]),center[i],radius[i]);

            if(radius[i] < minHollSize || radius[i] > maxHollSize)
                defect = true;
            qDebug()<<"x: "<<center[i].x<<" y: "<<center[i].y<<" radius : "<<radius[i];
        }
        if(i != 3)
            defect = true;

        totalProdocts ++;
        if(defect)
        {
            defectProdocts ++;
        }

        if(!ShowOriginalImage)
            memcpy(buffer[index], drawing.data,WIDTH * HEIGHT);
    }
    else
    {
        try
        {
            if(circleSize[index] <= 0)
                return;
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

            int i = 0;
            int defect = false;
            for(i = 0; i < contours.size(); i++)
            {

                if(!ShowOriginalImage)
                {
                    Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255) , rng.uniform(0, 255));
                    drawContours(drawing, contours , i , color , 2,8,hierarchy,0,Point());
                }
                minEnclosingCircle(Mat(contours[i]),center[i],radius[i]);

                if(radius[i] < minHollSize || radius[i] > maxHollSize)
                    defect = true;
                qDebug()<<"x: "<<center[i].x<<" y: "<<center[i].y<<" radius : "<<radius[i];
            }
            if(i != 2)
                defect = true;

            totalProdocts ++;
            if(defect)
            {
                defectProdocts ++;
            }

            if(!ShowOriginalImage)
                memcpy(bufferCircle[index], drawing.data,circleSize[index] * circleSize[index]);
            free(tmpBuffer);
        }catch(const GenericException &e)
        {
            qDebug()<<"1";
        }
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
    //    ui->graphicsView->scene()->
    if(imgUpdateView != NULL)
        free(imgUpdateView);

    if(showFullSizeImage || (!isTriggeMode))
        imgUpdateView = new QImage((uchar*)buffer[index], WIDTH, HEIGHT, QImage::Format_Indexed8);
    else
        imgUpdateView = new QImage((uchar*)bufferCircle[index], circleSize[index], circleSize[index], QImage::Format_Indexed8);

    scene->addPixmap(QPixmap::fromImage(*imgUpdateView));
    ui->graphicsView->show();

    ui->lblDefect->setText(QString::number(defectProdocts));
    ui->lblTotal->setText(QString::number(totalProdocts));
    ui->lblHistoryDefect->setText(QString::number(historyDefectProdocts));
    ui->lblHistoryTotal->setText(QString::number(historyTotalProdocts));
}

void MainWindow::on_pushButton_clicked()
{
    //    Dialog dialog(0,this);
    dialogConfig->show();
}

bool MainWindow::findDiameter(char *input,int index)
{
    bool result = true;
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
        for(int j = 10;j < 500; j++)
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

    int k = ((0.353553 * ( Rj - Uj) * 2 - 10));
    if(k < 0)
    {
        k = 10;
        Rj = HEIGHT / 2;
        Ri = WIDTH / 2;
        Ui = WIDTH / 2 - 10;
        Uj = HEIGHT / -10;
        result = false;
    }
    if(k % 2 != 0)
        k --;
    //    qDebug()<<"k = "<<k;
    circleSize[index] = k*2;


    int I ,J;

    for(int j = Rj-k; j < Rj+k; j++)
        for(int i = Ui-k; i < Ui+k; i++)
        {
            I = (i - Ui+k); J = (j - Rj+k);
            if((I + J * k*2) > WIDTH * HEIGHT  || (i + j * 800) > WIDTH * HEIGHT )
                return false;
            bufferCircle[index][I + J * k*2] = input[i + j * 800];
        }
    return result;
}

void MainWindow::on_btnTestGPIO_clicked()
{
    static bool first = true;
    if(first)
    {
        first = false;
        QtConcurrent::run(testTrigge);
    }
    else
    {
        stopTriggeTest();
        first = true;
    }

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"close";
    appClosed = 1;
    stopTriggeTest();
    camera->closeAllCameras();
    saveToHistory();
    event->accept();
}


void MainWindow::on_btnTestGPIO_2_clicked()
{
    saveToHistory();
}


void MainWindow::on_btnTestGPIO_3_clicked()
{
    loadHistory();
}

bool MainWindow::saveToHistory()
{
    QString today = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString now = QDateTime::currentDateTime().toString("hh-mm-ss");
    if(! QDir("history").exists())
        QDir().mkdir("history");

    if(! (QDir("history/" + today).exists()))
        QDir().mkdir("history/" + today);
    QString t = ("echo '" + QString::number(totalProdocts) + "_" + QString::number(defectProdocts) + "' >" + "history/" + today + "/" + today+ "_" + now);
    system(t.toUtf8());

}

bool MainWindow::loadHistory()
{
    int total = 0,defect = 0;
    QString today = QDateTime::currentDateTime().toString("yyyyMMdd");

    if(! QDir("history").exists())
        total = defect = 0;
    else if(! (QDir("history/" + today).exists()))
        total = defect = 0;
    else
    {
        QDir history("history/" + today);//qDebug()<<"123";
        QStringList list = history.entryList();
        foreach (QString fileName, list) {
            fileName = "history/" + today + "/" + fileName;
            QFile file(fileName);
            file.open(QFile::ReadOnly);
            QByteArray out = file.readAll();
            QString test = out.begin();

            QStringList values = test.split("_");
            if(values.count() == 2)
            {
                QString t = values.first();
                values.pop_front();
                QString d = values.first();
                total += (t.toInt());
                defect += d.toInt();
            }
        }
    }
    historyDefectProdocts = defect;
    historyTotalProdocts = total;

    ui->lblHistoryDefect->setText(QString::number(historyDefectProdocts));
    ui->lblHistoryTotal->setText(QString::number(historyTotalProdocts));

}
