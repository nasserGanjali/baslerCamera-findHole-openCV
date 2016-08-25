#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtConcurrentRun>
#include <QTimer>
#include <QFile>
#include <QDir>
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
#include <QPalette>
#include "basler.h"
#include "dialog.h"

#define WIDTH 800
#define HEIGHT 600

extern "C" void testTrigge();
extern "C" void stopTriggeTest();
extern "C" int GPIO_start();
extern "C" void singleShot(int value);
extern "C" int getGPIO();
extern "C" int isGPIOClosed;
extern "C" void setGPIO(int value);

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
    int lineLeft ,lineRight, lineUp, lineDown ;
    bool isTriggeMode;
    //    bool isHollFinding;
    bool ShowOriginalImage;
    bool showFullSizeImage;
    explicit MainWindow(QWidget *parent = 0);
    basler *camera;
    void getFrame();
    bool saveToHistory();
    bool loadHistory();
    void getInput();
    void loadConfig();
    void saveConfig();
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QTimer tmrGraphicView,tmrSave;
    void getFrameWhile();
    QGraphicsScene *scene;
    bool startCapture;

    Dialog *dialogConfig;
    dialogTestCamera *dialogCheckCamera;
    void findHoles(int index);
    QImage *imgUpdateView;
    char *buffer[10];
    char *bufferCircle[10];
    int circleSizeW[10],circleSizeH[10];
    int defectProdocts,totalProdocts,historyTotalProdocts,historyDefectProdocts;
    int lastProdoctIsDefect;
    int indexBuffer;
    bool findDiameter(char *input, int index);
    void Algorithm(int index);
    void addTmpFileToHistory();
    void saveImage(char *input);
    void updateGraphicHistory(int value);
    void initGraphicHistory();
private slots:
    void updateGraphicView();
    void closeEvent(QCloseEvent *event);
    //    void on_btnTestGPIO_clicked();
    void tmrSaveTicked();
    void on_actionImage_triggered();
    void on_actionExit_triggered();
    void on_actionCamera_triggered();
    void on_actionInternal_GPIO_triggered();

};

#endif // MAINWINDOW_H


