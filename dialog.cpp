#include "dialog.h"
#include "ui_dialog.h"
#include "mainwindow.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}
void Dialog::dialogData(float Pstat,float Uk1h,float Uk2h, float Uk10, float Uk20,float U4l,float U4m,float U4h, float U1l, float U1h, float U2l, float U2h)
{
    ui->lineEditCoef_1->setText(QString().number(Pstat));
    ui->lineEditCoef_2->setText(QString().number(Uk1h));
    ui->lineEditCoef_3->setText(QString().number(Uk2h));
    ui->lineEditCoef_4->setText(QString().number(Uk10));
    ui->lineEditCoef_5->setText(QString().number(Uk20));
    float K1 = Pstat/(Uk1h-Uk10);
    float K2 = Pstat/(Uk2h-Uk20);
    ui->lineEditCoef_6->setText(QString().number(K1));
    ui->lineEditCoef_7->setText(QString().number(K2));
    ui->lineEditCoef_8->setText(QString().number(U4l));
    ui->lineEditCoef_9->setText(QString().number(U4m));
    ui->lineEditCoef_10->setText(QString().number(U4h));
    ui->lineEditCoef_11->setText(QString().number(U1l));
    ui->lineEditCoef_12->setText(QString().number(U1h));
    ui->lineEditCoef_13->setText(QString().number(U2l));
    ui->lineEditCoef_14->setText(QString().number(U2h));
    //ui->lineEditCoef_3->setText(QString().number(Uk10));
    dU10 = Uk10;
    dU20 = Uk20;
    dU1h = Uk1h;
    dU2h = Uk2h;
}

void Dialog::on_pushButtonDownLoad_clicked()
{
    float *data = new float[4];
    data[0] = dU10;
    data[1] = dU20;
    data[2] = dU1h;
    data[3] = dU2h;
    emit MySetValueSignal(data);
}
