#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtConcurrentRun>
#include <QTimer>

#include "basler.h"

#define WIDTH 800
#define HEIGHT 600

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

    QImage *imgUpdateView;
    char *buffer[10];
    int indexBuffer;

private slots:
    void updateGraphicView();

};

#endif // MAINWINDOW_H
