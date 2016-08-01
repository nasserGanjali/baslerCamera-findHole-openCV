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

#include "basler.h"

#define WIDTH 800
#define HEIGHT 600

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void getFrame();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer tmrGraphicView;
    QGraphicsScene *scene;
    basler *camera;

    void findHoles();
    QImage *imgUpdateView;
    char *buffer[10];
    int indexBuffer;

private slots:
    void updateGraphicView();

};

#endif // MAINWINDOW_H
