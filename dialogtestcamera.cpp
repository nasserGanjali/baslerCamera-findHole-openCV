#include "dialogtestcamera.h"
#include "ui_dialogtestcamera.h"

dialogTestCamera::dialogTestCamera(QWidget *parent, MainWindow *main, basler *camera) :
    QDialog(parent),
    ui(new Ui::dialogTestCamera)
{
    Camera = camera;
    Main = main;
    ui->setupUi(this);
}

dialogTestCamera::~dialogTestCamera()
{
    delete ui;
}

void dialogTestCamera::on_btnCheckNetwork_clicked()
{
    QMessageBox *msg = new QMessageBox();

    QProcess proc;
    proc.start("/home/linaro/script/testConnection.sh");
    proc.waitForFinished();
    QByteArray res = proc.readAll();
    if(QString(res.begin()) == "0\n" )
        msg->setText("network is connected");
    else
        msg->setText("network is not connected !!!");
    msg->show();
    //    qDebug()<<res.begin();
}

void dialogTestCamera::on_btnConnect_clicked()
{
    QMessageBox *msg = new QMessageBox();

    QProcess proc;
    proc.start("/home/linaro/script/connect.sh");
    proc.waitForFinished();
    QByteArray res = proc.readAll();
    if(QString(res.begin()) == "0\n" )
        msg->setText("network in configured");
    else
        msg->setText("error in configuring network");
    msg->show();
}

//void dialogTestCamera::on_btnCheckCamera_clicked()
//{
//    QMessageBox *msg = new QMessageBox();

//    Main->appClosed = 1;
////    Main->stopTriggeTest();
//    Camera->closeAllCameras();
//    if(Camera->connect())
//        msg->setText("camera is connected");
//    else
//         msg->setText("camera is not connected !!!");
//    msg->show();
//}

void dialogTestCamera::on_btnOkey_clicked()
{
    this->close();
}
