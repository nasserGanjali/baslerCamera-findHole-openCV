#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent, MainWindow *mainwindow) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    Main = mainwindow;
    ui->scrKernel->setValue(Main->CV_kernelGain);
    ui->lblKernel->setText(QString::number(Main->CV_kernelGain));
    ui->scrMin->setValue(Main->CV_lowerd);
    ui->lblMin->setText(QString::number(Main->CV_lowerd));
    ui->ScrMax->setValue(Main->CV_upperb);
    ui->lblMax->setText(QString::number(Main->CV_upperb));
    ui->scrObjectThr->setValue(Main->objectThr);
    ui->lblObjectThr->setText(QString::number(Main->objectThr));
    ui->cbxShowFullSize->setChecked(Main->showFullSizeImage);
    ui->sbxMaxHoll->setValue(Main->maxHollSize);
    ui->sbxMinHoll->setValue(Main->minHollSize);
    ui->sbxRightLine->setValue(Main->lineRight);
    ui->sbxLeftLine->setValue(Main->lineLeft);
    ui->sbxUpLine->setValue(Main->lineUp);
    ui->sbxDownLine->setValue(Main->lineDown);

    ui->rbnCameraMode->setChecked(!Main->isTriggeMode);
//    ui->rbnHoll->setChecked(Main->isHollFinding);
    ui->cbxShowOriginalImage->setChecked(Main->ShowOriginalImage);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    Main->configDialogIsOpen = 0;
    event->accept();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_brnOK_clicked()
{
    Main->CV_lowerd = ui->scrMin->value();
    Main->CV_upperb = ui->ScrMax->value();
    Main->CV_kernelGain = ui->scrKernel->value();
    Main->objectThr = ui->scrObjectThr->value();
    Main->maxHollSize = ui->sbxMaxHoll->value();
    Main->minHollSize = ui->sbxMinHoll->value();
    Main->lineLeft = ui->sbxLeftLine->value();
    Main->lineRight = ui->sbxRightLine->value();
    Main->lineUp = ui->sbxUpLine->value();
    Main->lineDown = ui->sbxDownLine->value();
    if(ui->rbnCameraMode->isChecked())
        Main->isTriggeMode = false;
    else
        Main->isTriggeMode = true;

//    if(ui->rbnHoll->isChecked())
//        Main->isHollFinding = true;
//    else
//        Main->isHollFinding = false;

    if(ui->cbxShowOriginalImage->isChecked())
        Main->ShowOriginalImage = true;
    else
        Main->ShowOriginalImage = false;

    if(ui->cbxShowFullSize->isChecked())
        Main->showFullSizeImage = true;
    else
        Main->showFullSizeImage = false;

    Main->saveConfig();
    this->close();
}

void Dialog::on_btnCancel_clicked()
{
    this->close();
}

void Dialog::on_scrMin_valueChanged(int value)
{
    ui->lblMin->setText(QString::number(value));
}

void Dialog::on_ScrMax_valueChanged(int value)
{
    ui->lblMax->setText(QString::number(value));
}

void Dialog::on_scrKernel_valueChanged(int value)
{
    ui->lblKernel->setText(QString::number(value));
}

void Dialog::on_brnApply_clicked()
{
    Main->CV_lowerd = ui->scrMin->value();
    Main->CV_upperb = ui->ScrMax->value();
    Main->CV_kernelGain = ui->scrKernel->value();
    Main->objectThr = ui->scrObjectThr->value();
    Main->maxHollSize = ui->sbxMaxHoll->value();
    Main->minHollSize = ui->sbxMinHoll->value();
    Main->lineLeft = ui->sbxLeftLine->value();
    Main->lineRight = ui->sbxRightLine->value();
    Main->lineUp = ui->sbxUpLine->value();
    Main->lineDown = ui->sbxDownLine->value();
    if(ui->rbnCameraMode->isChecked())
        Main->isTriggeMode = false;
    else
        Main->isTriggeMode = true;

//    if(ui->rbnHoll->isChecked())
//        Main->isHollFinding = true;
//    else
//        Main->isHollFinding = false;

    if(ui->cbxShowOriginalImage->isChecked())
        Main->ShowOriginalImage = true;
    else
        Main->ShowOriginalImage = false;

    if(ui->cbxShowFullSize->isChecked())
        Main->showFullSizeImage = true;
    else
        Main->showFullSizeImage = false;
}

void Dialog::on_rbnCameraMode_clicked(bool checked)
{

}

void Dialog::on_scrObjectThr_valueChanged(int value)
{
    ui->lblObjectThr->setText(QString::number(value));
}


void Dialog::on_sbxRightLine_valueChanged(int arg1)
{

}

void Dialog::on_sbxLeftLine_valueChanged(int arg1)
{

}

void Dialog::on_sbxLeftLine_editingFinished()
{
    int m = ui->sbxRightLine->value() - ui->sbxLeftLine->value() + 1;
    if(m % 4 != 0 )
        ui->sbxLeftLine->setValue(ui->sbxLeftLine->value() + (m % 4));

    if(ui->sbxLeftLine->value() >= ui->sbxRightLine->value() )
        ui->sbxLeftLine->setValue(ui->sbxRightLine->value() - 4);
}

void Dialog::on_sbxRightLine_editingFinished()
{
    int m = ui->sbxRightLine->value() - ui->sbxLeftLine->value() + 1;
    qDebug()<<"m "<<m%4;
    if(m % 4 != 0 )
        ui->sbxRightLine->setValue(ui->sbxRightLine->value() + (4 - m % 4));

    if(ui->sbxLeftLine->value() >= ui->sbxRightLine->value() )
        ui->sbxRightLine->setValue(ui->sbxLeftLine->value() + 4);
}
