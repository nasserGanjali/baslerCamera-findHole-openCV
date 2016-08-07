#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtConcurrentRun>
#include <QTimer>
#include "dialogtestcamera.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/core/core.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <QThread>
#include <QTime>
#include <QCloseEvent>

#include "basler.h"
#include "dialog.h"

#define WIDTH 800
#define HEIGHT 600

extern "C" void testTrigge();
extern "C" void stopTriggeTest();
extern "C" int GPIO_start();
extern "C" void singleShot();

using namespace cv;

class Dialog;
namespace Ui {
class MainWindow;
}

class basler;
class dialogTestCamera;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    int CV_lowerd , CV_upperb;
    int CV_kernelGain;
    int objectThr;
    int triggerTimeout;
    int appClosed;
    int minHollSize,maxHollSize;
    bool isTriggeMode;
//    bool isHollFinding;
    bool ShowOriginalImage;
    bool showFullSizeImage;
    explicit MainWindow(QWidget *parent = 0);
     basler *camera;
    void getFrame();
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QTimer tmrGraphicView;
    void getFrameWhile();
    QGraphicsScene *scene;
    bool startCapture;

    Dialog *dialogConfig;
    dialogTestCamera *dialogCheckCamera;
    void findHoles(int index);
    QImage *imgUpdateView;
    char *buffer[10];
    char *bufferCircle[10];
    int circleSize[10];
    int defectProdocts,totalProdocts;
    int indexBuffer;
    bool findDiameter(char *input, int index);
    void Algorithm(int index);

private slots:
    void updateGraphicView();
    void closeEvent(QCloseEvent *event);
    void on_pushButton_clicked();
    void on_btnTestGPIO_clicked();
    void on_btnTestCamera_clicked();
};

#endif // MAINWINDOW_H


