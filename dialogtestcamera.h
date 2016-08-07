#ifndef DIALOGTESTCAMERA_H
#define DIALOGTESTCAMERA_H

#include <QDialog>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include "basler.h"
#include "mainwindow.h"

class basler;
class MainWindow;

namespace Ui {
class dialogTestCamera;
}

class dialogTestCamera : public QDialog
{
    Q_OBJECT

public:
    explicit dialogTestCamera(QWidget *parent = 0,MainWindow *main = 0,basler *camera = 0);
    ~dialogTestCamera();

private slots:
    void on_btnCheckNetwork_clicked();

    void on_btnConnect_clicked();

//    void on_btnCheckCamera_clicked();

private:
    MainWindow *Main;
    Ui::dialogTestCamera *ui;
    basler *Camera;
};

#endif // DIALOGTESTCAMERA_H
