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
    lastProdoctIsDefect = 0;
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

//    UjMax = 600;UjMin = 10;UiMax = 799;UiMin = 300;RjMax = 400;RjMin = 10 ;RiMax = 799;RiMin = 300;
    lineLeft = 300, lineRight = 500, lineUp = 400 , lineDown = 500;

    loadHistory();

    loadConfig();

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
        circleSizeW[i] = circleSizeH[i] = 0;

    tmrGraphicView.setInterval(10);
    connect(&tmrGraphicView,SIGNAL(timeout()),SLOT(updateGraphicView()));
    tmrGraphicView.start();

    tmrSave.setInterval(10000);
    connect(&tmrSave,SIGNAL(timeout()),SLOT(tmrSaveTicked()));
    tmrSave.start();

    //qDebug()<<"asdasd";
    dialogConfig = new Dialog(0,this);
    QtConcurrent::run(this,&MainWindow::getFrameWhile);
    //qDebug()<<"asdasd";
    GPIO_start();

    singleShot(0);

    //    getInput();
    QtConcurrent::run(this,&MainWindow::getInput);

    this->setGeometry(40,31,1850,1050);
    this->show();
//    qDebug()<<"test";

}

void MainWindow::loadConfig()
{
    QFile file("configuration.conf");
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            QString line = stream.readLine();
            if(line.contains("CV_lowerd="))
                CV_lowerd = line.mid(10,line.length()).toInt();
            if(line.contains("CV_upperb="))
                CV_upperb = line.mid(10,line.length()).toInt();
            if(line.contains("CV_kernelGain="))
                CV_kernelGain = line.mid(14,line.length()).toInt();
            if(line.contains("objectThr="))
                objectThr = line.mid(10,line.length()).toInt();
            if(line.contains("maxHollSize="))
                maxHollSize = line.mid(12,line.length()).toInt();
            if(line.contains("minHollSize="))
                minHollSize = line.mid(12,line.length()).toInt();
            if(line.contains("lineLeft="))
                lineLeft = line.mid(9,line.length()).toInt();
            if(line.contains("lineRight="))
                lineRight = line.mid(10,line.length()).toInt();
            if(line.contains("lineUp="))
                lineUp = line.mid(7,line.length()).toInt();
            if(line.contains("lineDown="))
                lineDown = line.mid(9,line.length()).toInt();
        }

        file.close();
    }
}

void MainWindow::saveConfig()
{
    QFile file("configuration.conf");
    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        stream << "CV_lowerd=" << QString::number(CV_lowerd) <<endl;
        stream << "CV_upperb=" << QString::number(CV_upperb) <<endl;
        stream << "CV_kernelGain=" << QString::number(CV_kernelGain) <<endl;
        stream << "objectThr=" << QString::number(objectThr) <<endl;
        stream << "maxHollSize=" << QString::number(maxHollSize) <<endl;
        stream << "minHollSize=" << QString::number(minHollSize) <<endl;
        stream << "lineLeft=" << QString::number(lineLeft) <<endl;
        stream << "lineRight=" << QString::number(lineRight) <<endl;
        stream << "lineUp=" << QString::number(lineUp) <<endl;
        stream << "lineDown=" << QString::number(lineDown) <<endl;
    }
    file.close();
}

void MainWindow::tmrSaveTicked()
{
    QString today = QDateTime::currentDateTime().toString("yyyyMMdd");
    //    QString now = QDateTime::currentDateTime().toString("hh-mm-ss");
    if(! QDir("history").exists())
        QDir().mkdir("history");

    if(! (QDir("history/" + today).exists()))
        QDir().mkdir("history/" + today);
    QString t = ("echo '" + QString::number(totalProdocts) + "_" + QString::number(defectProdocts) + "' >" + "history/" + today + "/" + today+ "_tmp");
    //    qDebug()<<t;
    system(t.toUtf8());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionImage_triggered()
{
    dialogConfig->show();
}

void MainWindow::on_actionCamera_triggered()
{
    dialogCheckCamera->show();
}

void MainWindow::on_actionInternal_GPIO_triggered()
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

void MainWindow::getInput()
{
    while(!appClosed)
    {
        u_int16_t v = getGPIO();
        qDebug()<<v;

        if(v == 3)
        {
            singleShot(2); // a prodoct is detected shot camera trigge
            continue;
        }

        if(lastProdoctIsDefect)
            singleShot(1);
        else
            qDebug()<<"is not defect";

    }
}

void MainWindow::getFrameWhile()
{
    while(!appClosed)
    {
        //        qDebug()<<"time :"<<QTime::currentTime().toString();
        getFrame();
    }
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
        {
             stopTriggeTest();
             qDebug()<<"bad shot !!!";
//                QTimer::setSingleShot(1);
            singleShot(2);
            return;
        }
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
//        totalProdocts ++;

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
//    findDiameter(test,index);
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
            lastProdoctIsDefect = 1;
        }else
        {
            lastProdoctIsDefect = 0;
        }

        if(!ShowOriginalImage)
            memcpy(buffer[index], drawing.data,WIDTH * HEIGHT);
    }
    else
    {
        try
        {
//            qDebug()<<"sizeW:"<<circleSizeW[index];
            char *tmpBuffer = (char *)malloc(circleSizeW[index] * circleSizeH[index]);
            memcpy(tmpBuffer,bufferCircle[index],circleSizeW[index] * circleSizeH[index]);
            cv::Mat img(circleSizeH[index],circleSizeW[index],CV_8U,tmpBuffer);
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
            int defect = false;//qDebug()<<"x1: ";
            for(i = 0; i < contours.size(); i++)
            {
//                qDebug()<<"x: ";
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
                lastProdoctIsDefect = 1;
            }else
            {
                lastProdoctIsDefect = 0;
            }

            if(!ShowOriginalImage)
                memcpy(bufferCircle[index], drawing.data,circleSizeW[index] * circleSizeH[index]);
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

    if(!isTriggeMode || showFullSizeImage)
    {
        for(int j = 0, i = lineLeft; j < HEIGHT; j++)
            buffer[index][i + j * WIDTH] = 255;

        for(int j = 0, i = lineRight; j < HEIGHT; j++)
            buffer[index][i + j * WIDTH] = 255;

        for(int i = 0, j = lineUp; i < WIDTH; i++)
            buffer[index][i + j * WIDTH] = 255;

        for(int i = 0, j = lineDown; i < WIDTH; i++)
            buffer[index][i + j * WIDTH] = 255;
    }

    if(showFullSizeImage || (!isTriggeMode))
        imgUpdateView = new QImage((uchar*)buffer[index], WIDTH, HEIGHT, QImage::Format_Indexed8);
    else
    {
        imgUpdateView = new QImage((uchar*)bufferCircle[index], circleSizeW[index], circleSizeH[index], QImage::Format_Indexed8);
    }
    scene->addPixmap(QPixmap::fromImage(*imgUpdateView));
    ui->graphicsView->show();

    ui->lblDefect->setText(QString::number(defectProdocts));
    ui->lblTotal->setText(QString::number(totalProdocts));
    ui->lblHistoryDefect->setText(QString::number(historyDefectProdocts));
    ui->lblHistoryTotal->setText(QString::number(historyTotalProdocts));
}

/*
 * UjMax = 600;UjMin = 10;UiMax = 799;UiMin = 300;RjMax = 400;RjMin = 10 ;RiMax = 799;RiMin = 300;
 */

bool MainWindow::findDiameter(char *input,int index)
{
    //UjMax = 600;UjMin = 10;UiMax = 500;UiMin = 0;RjMax = 400;RjMin = 10 ;RiMax = 500;RiMin = 0;
    bool result = true;

    //    qDebug()<<"k = "<<k;

    circleSizeW[index] = lineRight - lineLeft +  1;
    circleSizeH[index] = lineDown - lineUp  + 1;

    int I ,J;
    int s =0;
    for(int j = lineUp; j <= lineDown; j++)
        for(int i = lineLeft; i <= lineRight; i++)
        {
            I = (i - lineLeft); J = (j - lineUp);

            bufferCircle[index][I + J * circleSizeW[index]] = input[i + j * 800];
            s++;
        }
    qDebug()<<"s :"<<s;
    return result;
}

//void MainWindow::on_btnTestGPIO_clicked()
//{
//    static bool first = true;
//    if(first)
//    {
//        first = false;
//        QtConcurrent::run(testTrigge);
//    }
//    else
//    {
//        stopTriggeTest();
//        first = true;
//    }

//}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"close";
    appClosed = 1;
    isGPIOClosed = 1;
    stopTriggeTest();
    camera->closeAllCameras();
    saveToHistory();
    singleShot(0);
    event->accept();
}

void MainWindow::on_actionExit_triggered()
{
    closeEvent(0);
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

    // remove tmp file
    QFile tmp("history/" + today + "/" + today+ "_tmp");
    tmp.remove();
}

bool MainWindow::loadHistory()
{
    addTmpFileToHistory();
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

void MainWindow::addTmpFileToHistory()
{
    int total = 0,defect = 0;
    QString today = QDateTime::currentDateTime().toString("yyyyMMdd");

    if(! QDir("history").exists())
        total = defect = 0;
    else if(! (QDir("history/" + today).exists()))
        total = defect = 0;
    else
    {
        QDir history("history/" + today);
        QStringList list = history.entryList();
        foreach (QString fileName, list) {
            if(!fileName.contains("tmp"))
                continue;
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
            file.remove();
        }
    }

    if(total != 0 || defect != 0)
    {
        QString now = QDateTime::currentDateTime().toString("hh-mm-ss");
        if(! QDir("history").exists())
            QDir().mkdir("history");

        if(! (QDir("history/" + today).exists()))
            QDir().mkdir("history/" + today);
        QString t = ("echo '" + QString::number(total) + "_" + QString::number(defect) + "' >" + "history/" + today + "/" + today+ "_" + now);
        system(t.toUtf8());

    }
}
