#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "mainwindow.h"

class MainWindow;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0,MainWindow *mainwindow = NULL);
    ~Dialog();

private slots:
    void on_brnOK_clicked();

    void on_btnCancel_clicked();

    void on_scrMin_valueChanged(int value);

    void on_ScrMax_valueChanged(int value);

    void on_scrKernel_valueChanged(int value);

    void on_brnApply_clicked();

    void on_rbnCameraMode_clicked(bool checked);

    void on_scrObjectThr_valueChanged(int value);

private:
    Ui::Dialog *ui;
    MainWindow *Main;
};

#endif // DIALOG_H
