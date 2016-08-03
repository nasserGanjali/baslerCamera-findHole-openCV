#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtConcurrentRun>
#include <QTimer>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/core/core.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <QThread>

#include "basler.h"
#include "dialog.h"

#define WIDTH 800
#define HEIGHT 600

using namespace cv;

class Dialog;
namespace Ui {
class MainWindow;
}

class basler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    int CV_lowerd , CV_upperb;
    int CV_kernelGain;
    int objectThr;
    bool isTriggeMode;
//    bool isHollFinding;
    bool ShowOriginalImage;
    bool showFullSizeImage;
    explicit MainWindow(QWidget *parent = 0);
    void getFrame();
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QTimer tmrGraphicView;
    void getFrameWhile();
    QGraphicsScene *scene;
    basler *camera;
    bool startCapture;

    Dialog *dialogConfig;
    void findHoles(int index);
    QImage *imgUpdateView;
    char *buffer[10];
    char *bufferCircle[10];
    int circleSize[10];
    int indexBuffer;
    void findDiameter(char *input, int index);

private slots:
    void updateGraphicView();

    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
