#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "request.h"
#include "answer.h"
#include "dialog.h"
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QString>
#include <QStringRef>
#include "QDebug"
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QMenu>
#define private public

static QString styleSheetCalibrationDefault =           "QLabel{background-color :rgba(255,0,0,100);border-radius:7px;}";
static QString styleSheetCalibrationOk =                "QLabel{background-color :rgba(0,255,0,200);border-radius:7px;}";
static QString styleSheetCalibrationBad =               "QLabel{background-color :rgba(255,0,0,200);border-radius:7px;}";
static QString styleSheetCalibrationLineEditDefault =   "QLineEdit{background-color :rgba(0,0,0,0);}";
static QString styleSheetCalibrationLineEditBad =       "QLineEdit{background-color :rgba(255,0,0,50);}";
static QString styleSheetCalibrationLineEditGood =      "QLineEdit{background-color :rgba(0,255,0,50);}";
static QString styleSheetCalibrationSpinBoxDefault =    "QSpinBox{background-color :rgba(0,0,0,0);}";
static QString styleSheetCalibrationSpinBoxBad =        "QSpinBox{background-color :rgba(255,0,0,50);}";
static QString styleSheetCalibrationSpinBoxGood =       "QSpinBox{background-color :rgba(0,255,0,50);}";
static QString styleSheetCalibrationComboBoxDefault =   "QComboBox{}";
static QString styleSheetCalibrationComboBoxBad =       "QComboBox{background-color :rgba(255,0,0,50);}";
static QString styleSheetCalibrationComboBoxGood =      "QComboBox{background-color :rgba(0,255,0,50);}";


//QThread::msleep(5000);
static bool PortOpen;
static bool answerStart = false;
static bool answerIsGet = false;
static int inBytesExpected;
static int NumFunc;
static unsigned char ReqData[2] = {0x04,0x02};
static unsigned char ReqAddr[5] = {0x0f,0x02,0x06,0x01,0xfd};
static int nSymAnsGet = 0;
//static answer b(inBytesExpected);
static char *ansGet = new char[inBytesExpected];
static int prLength = 0;
static unsigned char *data91;
static char *h = new char[50];
static int requestIsSend = 0;
static bool requset1IsSend = 0;
static bool requset2IsSend = 0;
static bool requset3IsSend = 0;
static bool requset4IsSend = 0;
static bool requset5IsSend = 0;
static int timerSendRequest = 150;
static int numberRow = 0;
static QStandardItemModel *model = new QStandardItemModel;
static QComboBox *comboBoxAddress;
static int countIndicateCalibration = 0;
static int lastFunc;
static bool lastFrame;
static int pLo = 0;
static int countTimerFinc3 = 0;
static bool allRequestIsSend = false;

void ResetAddress()
{
    comboBoxAddress->clear();
    comboBoxAddress->addItem("00",0);
    comboBoxAddress->addItem("01",1);
    comboBoxAddress->addItem("02",2);
    comboBoxAddress->addItem("03",3);
    comboBoxAddress->addItem("04",4);
    comboBoxAddress->addItem("05",5);
    comboBoxAddress->addItem("06",6);
    comboBoxAddress->addItem("07",7);
    comboBoxAddress->addItem("08",8);
    comboBoxAddress->addItem("09",9);
    comboBoxAddress->addItem("0a",10);
    comboBoxAddress->addItem("0b",11);
    comboBoxAddress->addItem("0c",12);
    comboBoxAddress->addItem("0d",13);
    comboBoxAddress->addItem("0e",14);
    comboBoxAddress->addItem("0f",15);
    comboBoxAddress->setCurrentIndex(14);
}
float* findMx(QString hMidle)
{
    float* Umx = new float [3];
    QStringRef subString;
    int k = 0;
    int len = 0;
    int countVar = 0;
    int pos = 0;
    float f;
    float mxU1 = 0, mxU2 = 0, mxU4 = 0;
    for(k=0;k<hMidle.length();k++)
    {
        if(hMidle[k] == 0x0009)
        {
            subString = QStringRef(&hMidle, pos, len);
            if (countVar%3==0) {
                f = subString.toFloat();
                mxU1 = mxU1*((int)countVar/3)+f;
                mxU1 = mxU1/(int)(countVar/3+1);
            }
            if (countVar%3==1) {
                f = subString.toFloat();
                mxU2 = mxU2*((int)countVar/3)+f;
                mxU2 = mxU2/(int)(countVar/3+1);
            }
            if (countVar%3==2) {
                f = subString.toFloat();
                mxU4 = mxU4*((int)countVar/3)+f;
                mxU4 = mxU4/(int)(countVar/3+1);
            }
            pos =k+1;
            len =0;
            countVar++;
            continue;
        }
        len++;
    }

    Umx[0] = mxU1;
    Umx[1] = mxU2;
    Umx[2] = mxU4;
    return Umx;

}
void removeFile(QString address)
{
    QString fileDir;
    fileDir= QString("data/dataCalibrationHigh_")+address+QString(".txt");
    QFile(fileDir).remove();
    fileDir= QString("data/dataCalibrationLow_")+address+QString(".txt");
    QFile(fileDir).remove();
    fileDir= QString("data/dataCalibrationMiddle_")+address+QString(".txt");
    QFile(fileDir).remove();
}
void addFunctions(QComboBox *func)
{
    func->addItem("0.Считать уникальный идентификатор",0);
    func->addItem("3.Считать ток и значение четырех динамических переменных",3);
    //func->addItem("Function 13",13);
    //func->addItem("Function 35",35);
    func->addItem("36.Установить верхнее значение диапазона",36);
    func->addItem("37.Установить нижнее значение диапазона ",37);
    func->addItem("43.Установить нуль первичной переменной",43);
    //func->addItem("Function 45",45);
    func->addItem("51.Чтение нескольких регистров",51);
    func->addItem("91.Записть нескольких регистров",91);
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    setDialog(new Dialog),
    ui(new Ui::MainWindow),
    serial(new QSerialPort(this)),    
    timer(new QTimer(this)),
    timerFunction3(new QTimer(this)),
    timerFunction3Loop(new QTimer(this)),
    timerFindDevice(new QTimer(this)),
    timerCalibration(new QTimer(this))
    {
        ui->setupUi(this);
        ui->buttonClose->setEnabled(false);
        ui->buttonFindDevice->setEnabled(false);
        ui->buttonSend->setEnabled(!PortOpen);
        ui->checkEnTextBrows->setCheckState(Qt::Checked);
        ui->buttonSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        ui->lineEditSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        //ui->tabWidget->setTabEnabled(0,false);

        ui->label_3->setStyleSheet(styleSheetCalibrationDefault);
        ui->label_4->setStyleSheet(styleSheetCalibrationDefault);
        ui->label_5->setStyleSheet(styleSheetCalibrationDefault);
        updateCom();

        QString arg1 = ui->lineEdit_2->text();
        QByteArray text =QByteArray(arg1.toLocal8Bit());
        text.remove(2,1);
        QByteArray hex = QByteArray::fromHex(text);
        ReqData[0] = char(hex[0]);
        ReqData[1] = char(hex[1]);
        ui->spinData91->setValue(1);
        ui->checkDevice_1->setCheckState(Qt::Checked);
        addFunctions(ui->comboBoxFunc);


        ui->comboBoxSetAddress->clear();
        ui->comboBoxSetAddress->addItem("00",0);
        ui->comboBoxSetAddress->addItem("01",1);
        ui->comboBoxSetAddress->addItem("02",2);
        ui->comboBoxSetAddress->addItem("03",3);
        ui->comboBoxSetAddress->addItem("04",4);
        ui->comboBoxSetAddress->addItem("05",5);
        ui->comboBoxSetAddress->addItem("06",6);
        ui->comboBoxSetAddress->addItem("07",7);
        ui->comboBoxSetAddress->addItem("08",8);
        ui->comboBoxSetAddress->addItem("09",9);
        ui->comboBoxSetAddress->addItem("0a",10);
        ui->comboBoxSetAddress->addItem("0b",11);
        ui->comboBoxSetAddress->addItem("0c",12);
        ui->comboBoxSetAddress->addItem("0d",13);
        ui->comboBoxSetAddress->addItem("0e",14);
        ui->comboBoxSetAddress->addItem("0f",15);

        ui->comboBoxSetMode->addItem("HART",2);
        ui->comboBoxSetMode->addItem("RS-485",4);

        comboBoxAddress = ui->comboBoxAddress;
        connections();
        ResetAddressInit();
        timer->stop();
        timerFunction3->stop();
        timerFunction3Loop->stop();
        timerFindDevice->stop();
        downloadSettings();
        getRequestAddr(ui->checkLongFrame->checkState());
        ui->menutest->addAction("О программе",this,SLOT(aboutHartTester()));
        ui->menutest->addAction("O Qt",this,SLOT(aboutQt()));


    }
MainWindow::~MainWindow()
{
    saveSettings();
    serial->close();
    delete ui;
}
void MainWindow::connections()
{

    connect(ui->actionDialog,            &QAction::triggered,  &setDialog,&Dialog::show);

    connect(serial,                     &QSerialPort::readyRead,this,   &MainWindow::readData);             //slot recieve data from COM port
    connect(timer,                      &QTimer::timeout,       this,   &MainWindow::showTime);             //slot timer for sending loop request
    connect(timerFindDevice,            &QTimer::timeout,       this,   &MainWindow::sendFindRequest);      //slot timer for sending find request
    connect(ui->buttonResetAddr,        &QPushButton::clicked,  this,   &MainWindow::ResetAddressInit);     //slot button for reset combobox address
    connect(ui->pushButton,             &QPushButton::clicked,  this,   &MainWindow::connectCOM);           //slot button for connection COM port
    connect(ui->buttonClose,            &QPushButton::clicked,  this,   &MainWindow::closeCOM);             //slot button for closing COM port
    connect(ui->buttonSend,             &QPushButton::clicked,  this,   &MainWindow::sendRequest);          //slot button for closing COM port
    connect(ui->buttonClear,            &QPushButton::clicked,  this,   &MainWindow::clearText);            //slot button for clear developer TextEEdit
    connect(ui->buttonTimerStart,       &QPushButton::clicked,  this,   &MainWindow::startLoop);            //slot button start timer sending loop request
    connect(ui->buttonTimerStop,        &QPushButton::clicked,  this,   &MainWindow::stopLoop);             //slot button stop timer sending loop request
    connect(ui->pushButton_7,           &QPushButton::clicked,  this,   &MainWindow::create91Request);      //slot button create request drom data for request91
    connect(ui->pushButton_10,          &QPushButton::clicked,  this,   &MainWindow::stopLoopFunction3);    //slot button start timer for sending loop request for tab Function3
    connect(ui->pushButton_11,          &QPushButton::clicked,  this,   &MainWindow::clearTable);           //slot button clear table for tab Function 3
    connect(ui->buttonFindDevice,       &QPushButton::clicked,  this,   &MainWindow::findDevice);           //slot button start timer for sending find request
    connect(ui->buttonFunction3Send,    &QPushButton::clicked,  this,   &MainWindow::Function3Send);        //slot button for sending request for tab Function3
    connect(ui->buttonFunction3Loop,    &QPushButton::clicked,  this,   &MainWindow::startLoopFunction3);   //slot button start timer for sending loop request for tab Function3
    connect(ui->buttonSpan,             &QPushButton::clicked,  this,   &MainWindow::spanRequest);          //slot button for sending Span request for tab Calibration
    connect(ui->buttonZero,             &QPushButton::clicked,  this,   &MainWindow::zeroRequest);          //slot button for sending Zero request for tab Calibration
    connect(ui->buttonZeroFirstVar,     &QPushButton::clicked,  this,   &MainWindow::zeroFirstVarRequest);  //slot button for sending request zero first variable for tab Calibration
    connect(ui->linePassword,           &QLineEdit::textEdited, this,   &MainWindow::checkPassword);
    connect(ui->buttonSetAddress,       &QPushButton::clicked,  this,   &MainWindow::setAddress);
    connect(ui->buttonSetMode,          &QPushButton::clicked,  this,   &MainWindow::setMode);
    connect(ui->buttonSetMaxValue,      &QPushButton::clicked,  this,   &MainWindow::setMaxValue);
    connect(ui->buttonSetMaxValue_2,    &QPushButton::clicked,  this,   &MainWindow::setMaxValue_2);
    connect(ui->buttonMovingAverage_1,  &QPushButton::clicked,  this,   &MainWindow::setMovingAverage_1);
    connect(ui->buttonMovingAverage_2,  &QPushButton::clicked,  this,   &MainWindow::setMovingAverage_2);
    connect(ui->buttonSetA_40,          &QPushButton::clicked,  this,   &MainWindow::setA_40);
    connect(ui->buttonSetA_41,          &QPushButton::clicked,  this,   &MainWindow::setA_41);
    connect(ui->buttonSetA_42,          &QPushButton::clicked,  this,   &MainWindow::setA_42);
    connect(ui->buttonGetAddress,       &QPushButton::clicked,  this,   &MainWindow::getAddress);
    connect(ui->buttonGetMode,          &QPushButton::clicked,  this,   &MainWindow::getMode);
    connect(ui->buttonGetMaxValue,      &QPushButton::clicked,  this,   &MainWindow::getMaxValue);
    connect(ui->buttonGetMaxValue_2,    &QPushButton::clicked,  this,   &MainWindow::getMaxValue_2);
    connect(ui->buttonGetMovingAverage_1,&QPushButton::clicked, this,   &MainWindow::getMovingAverage_1);
    connect(ui->buttonGetMovingAverage_2,&QPushButton::clicked, this,   &MainWindow::getMovingAverage_2);
    connect(ui->buttonGetA_40,          &QPushButton::clicked,  this,   &MainWindow::getA_40);
    connect(ui->buttonGetA_41,          &QPushButton::clicked,  this,   &MainWindow::getA_41);
    connect(ui->buttonGetA_42,          &QPushButton::clicked,  this,   &MainWindow::getA_42);
    connect(ui->buttonSetFixedCurrent,  &QPushButton::clicked,  this,   &MainWindow::setValueFixedCurrent);
    connect(ui->buttonGetCurrent,       &QPushButton::clicked,  this,   &MainWindow::getCurrent);
    connect(ui->buttonGetPressue,       &QPushButton::clicked,  this,   &MainWindow::getPressue);
    connect(ui->buttonUpdateCom,        &QPushButton::clicked,  this,   &MainWindow::updateCom);
    connect(ui->buttonFindCoef,         &QPushButton::clicked,  this,   &MainWindow::findCoef);
    connect(ui->buttonClearCalibration, &QPushButton::clicked,  this,   &MainWindow::clearCalibrationData);
}
void MainWindow::downloadSettings()
{
    QSettings settings("STLab","HartTester");
    ui->tabWidget->setCurrentIndex      (settings.value("/Parameters/CurrentTab").toInt());
    ui->comboBoxAddress->setCurrentText (settings.value("/Parameters/CurrentFunc").toString());
    ui->comboBoxFunc->setCurrentIndex   (settings.value("/Parameters/CurrentIndex").toInt());
    ui->spinBox->setValue               (settings.value("/Parameters/CurrentPreamble").toInt());
    if(                                  settings.value("/Parameters/CurrentFrame").toBool())
    {
        ui->checkLongFrame->setCheckState(Qt::Checked);
    }
    else
    {
        ui->checkLongFrame->setCheckState(Qt::Unchecked);
    }
    ui->spinData91->setValue(           settings.value("/Parameters/CurrentData91").toInt());
    ui->lineAddr91->setText(            settings.value("/Parameters/CurrentAddr91").toString());
    ui->lineEdit_2->setText(            settings.value("/Parameters/CurrentAddr51").toString());
    ui->comboBoxAddressFunc3_1->setCurrentIndex(settings.value("/Parameters/AddrFunc3_1").toInt());
    ui->comboBoxAddressFunc3_2->setCurrentIndex(settings.value("/Parameters/AddrFunc3_2").toInt());
    ui->comboBoxAddressFunc3_3->setCurrentIndex(settings.value("/Parameters/AddrFunc3_3").toInt());
    ui->comboBoxAddressFunc3_4->setCurrentIndex(settings.value("/Parameters/AddrFunc3_4").toInt());
    ui->comboBoxAddressFunc3_5->setCurrentIndex(settings.value("/Parameters/AddrFunc3_5").toInt());
}
void MainWindow::saveSettings()
{
    QSettings settings("STLab","HartTester");
    settings.setValue("/Parameters/CurrentAddress", ui->comboBoxAddress->currentText());
    settings.setValue("/Parameters/CurrentFunc",    ui->comboBoxFunc->currentIndex());
    settings.setValue("/Parameters/CurrentPreamble",ui->spinBox->text());
    settings.setValue("/Parameters/CurrentFrame",   ui->checkLongFrame->checkState());
    settings.setValue("/Parameters/CurrentTab",     ui->tabWidget->currentIndex());
    settings.setValue("/Parameters/CurrentData91",  ui->spinData91->value());
    settings.setValue("/Parameters/CurrentAddr91",  ui->lineAddr91->text());
    settings.setValue("/Parameters/CurrentAddr51",  ui->lineEdit_2->text());
    settings.setValue("/Parameters/AddrFunc3_1",    ui->comboBoxAddressFunc3_1->currentIndex());
    settings.setValue("/Parameters/AddrFunc3_2",    ui->comboBoxAddressFunc3_2->currentIndex());
    settings.setValue("/Parameters/AddrFunc3_3",    ui->comboBoxAddressFunc3_3->currentIndex());
    settings.setValue("/Parameters/AddrFunc3_4",    ui->comboBoxAddressFunc3_4->currentIndex());
    settings.setValue("/Parameters/AddrFunc3_5",    ui->comboBoxAddressFunc3_5->currentIndex());
    //settings.setValue("/Parameters/SetA41",         ui->lineEditSetA_41->text().toFloat());
}
void MainWindow::ResetAddressInit()
{
    comboBoxAddress = ui->comboBoxAddress;
    ResetAddress();
    comboBoxAddress = ui->comboBoxAddressFunc3_1;
    ResetAddress();
    comboBoxAddress = ui->comboBoxAddressFunc3_2;
    ResetAddress();
    comboBoxAddress = ui->comboBoxAddressFunc3_3;
    ResetAddress();
    comboBoxAddress = ui->comboBoxAddressFunc3_4;
    ResetAddress();
    comboBoxAddress = ui->comboBoxAddressFunc3_5;
    ResetAddress();
}
void MainWindow::createRequestOut(bool tr = false)
{
    request a;
    a.setLongFrame(ui->checkLongFrame->checkState());
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(ReqAddr);
    switch (NumFunc) {
        case 0:
            a.setLongFrame(0);
            a.function0();

            break;
        case 3:a.function3();break;
        case 13:a.function13();break;
        case 35:a.function35();break;
        case 36:a.function36();break;
        case 37:a.function37();break;
        case 43:a.function43();break;
        case 45:a.function45();break;
        case 51:a.function51(ReqData);break;
        case 91:a.function91(data91,int(ui->spinData91->value())+1);break;
    }
    if(tr) serial->write(a.getRequest(),a.getRequestLength());
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
}
void MainWindow::getRequestAddr(bool b)
{
    if(!b)
    {
        // textAddr = ui->lineAddrShort->text();
        QString textAddr = ui->comboBoxAddress->currentText();
        QByteArray text =QByteArray(textAddr.toLocal8Bit());
        QByteArray hex = QByteArray::fromHex(text);
        ReqAddr[0] = char(hex[0]);
        createRequestOut();
    }
    else
    {
        QString textAddr = ui->comboBoxAddress->currentText();
        QByteArray text =QByteArray(textAddr.toLocal8Bit());
        QByteArray hex = QByteArray::fromHex(text);
        ReqAddr[0] = 0x0f;
        ReqAddr[1] = 0x02;
        ReqAddr[2] = char(hex[0]);
        ReqAddr[3] = 0xfd;
        ReqAddr[4] = 0xef;
        createRequestOut();
    }
}
void MainWindow::changedInByteExpected(){
    switch (NumFunc) {
        case 0:
            inBytesExpected = int(ui->spinBox->value())+19+2*int(ui->checkLongFrame->checkState());
            break;
        case 3:
            inBytesExpected = int(ui->spinBox->value())+31+2*int(ui->checkLongFrame->checkState());
            break;
        case 36:
            inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
            break;
        case 37:
            inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
            break;
        case 43:
            inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
            break;
        case 51:
            inBytesExpected = int(ui->spinBox->value())+7+int(ReqData[1])+2*int(ui->checkLongFrame->checkState());
            break;
        case 91:
            inBytesExpected = int(ui->spinBox->value())+9+2*int(ui->checkLongFrame->checkState());
        break;
    }
}
void MainWindow::showHideTableRow(QString r, bool t){
    int numberRow = 0;
    numberRow = model->rowCount();
    if(!t && numberRow != 0)
    {
        for(int i = 0; i<numberRow; i++)
        {
            QString g = model->item(i,0)->text();
            //QString textAddr = r->text();
            if(r == g && numberRow != 0)
            {
                ui->tableView->hideRow(i);
            }
        }

    }
    if(t && numberRow != 0)
    {
        for(int i = 0; i<numberRow; i++)
        {
            QString g = model->item(i,0)->text();
            //QString textAddr = r->text();
            if(r == g)
            {
                ui->tableView->showRow(i);
            }
        }
    }
}
void MainWindow::updateCom()
{
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
                ui->comboBox->addItem(info.portName());
}
void MainWindow::aboutHartTester()
{
    QMessageBox::about(this, "О программе", "HartTester Program \nVer. 0.20181101a");
}
void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}
void MainWindow::connectCOM()//connect
{
    serial->open(QSerialPort::ReadWrite);
    if (serial->portName() != ui->comboBox->currentText())
    {
          serial->close();
          serial->setPortName(ui->comboBox->currentText());
    }
    serial->setBaudRate(QSerialPort::Baud1200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::OddParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if(serial->isOpen()){
        ui->pushButton->setEnabled(false);
        ui->buttonClose->setEnabled(true);
        ui->buttonFindDevice->setEnabled(true);
        ui->textEdit->setText(" ");
    }
    else {
        ui->textEdit->setText("Not open");
    }
    //serial->setFlowControl(QSerialPort::SoftwareControl);
}
void MainWindow::closeCOM()//close
{
    if(serial->isOpen()){
        ui->pushButton->setEnabled(true);
        ui->buttonClose->setEnabled(false);
    }
    timer->stop();
    serial->close();
}
void MainWindow::sendRequest()//send
{

    //QThread::msleep(270);
    if(serial->isOpen()){
        ui->lineEditStatus->clear();
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(0,255,0);}");
    }
    else {
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
    }
    serial->setRequestToSend(0);
    QString textRequest = ui->lineRequest->text();
    QByteArray text =QByteArray(textRequest.toLocal8Bit());
    int j = 0;
    for(j = 2; j<text.length(); j+=2)
    {
        text.remove(j,1);
    }
    QByteArray hex = QByteArray::fromHex(text);
    serial->write(hex,hex.length());
    answerIsGet = false;
    if(serial->isOpen()){
        ui->lineEditStatus->setText("ОЖИДАНИЕ ОТВЕТА");
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(0,255,0);color:white;font:bold}");
    }
    if(ui->checkAltView->checkState()){
        ui->textEdit->clear();
    }
    QTextEdit *textRequestOut;
    switch (int(ui->tabWidget->currentIndex())) {
        case 0: textRequestOut = ui->textEdit;break;
        //case 1: textRequestOut = ui->textEditFunction3;break;
        default: textRequestOut = ui->textEdit;break;
    }
    if(1 || int(ui->tabWidget->currentIndex()) == 0 || int(ui->tabWidget->currentIndex()) == 2){
        textRequestOut->setTextBackgroundColor(QColor(255,255,255));
        textRequestOut->setTextColor(QColor(0,0,0));
        textRequestOut->insertPlainText(QString("\n")+textRequest+QString(" "));
    }

    if(ui->comboBoxFunc->currentData().toInt() == 91){
        ui->buttonSend->setEnabled(false);
        ui->buttonTimerStart->setEnabled(false);
        ui->pushButton_7->setEnabled(true);
    }
}
void MainWindow::readData()//read data
{

    QStandardItem *item;
    QStringList horizontalHeader;
    horizontalHeader.append("Адрес");
    horizontalHeader.append("Значение тока, мА");
    horizontalHeader.append("Напряжение U1");
    horizontalHeader.append("Напряжение U2");
    horizontalHeader.append("Напряжение U3");
    horizontalHeader.append("Напряжение U4");
    horizontalHeader.append("CRC");
    horizontalHeader.append("Файл");
    model->setHorizontalHeaderLabels(horizontalHeader);

    int bytesAvaible = int(serial->bytesAvailable());
    char *buf = new char[bytesAvaible];
    serial->read(buf,bytesAvaible);
    serial->clear(QSerialPort::Input);
    int i = 0;
    QTextEdit *text_out;
    switch (int(ui->tabWidget->currentIndex()))
    {
        case 0: text_out = ui->textEdit;break;
        //case 1: text_out = ui->textEditFunction3;break;
        default: text_out = ui->textEdit;break;
    }
    if(int(ui->tabWidget->currentIndex())!=1)
    {
        text_out->setTextBackgroundColor(QColor(0,0,0));
        text_out->setTextColor(QColor(255,255,255));
        text_out->insertPlainText(QByteArray(buf,bytesAvaible).toHex());
    }
    for(i=0; i<bytesAvaible; i++){
        if(answerStart){
            ansGet[nSymAnsGet] = buf[i];
            nSymAnsGet++;
            if(nSymAnsGet == inBytesExpected){
                QByteArray outText = QByteArray(ansGet,nSymAnsGet).toHex();
                int m = 0;
                for(m=2;m<outText.length();m=m+3){
                    outText = outText.insert(m," ");
                }
                answer b(nSymAnsGet);
                b.createAnswer(ansGet,nSymAnsGet);
                b.analysis();
                if(b.CrcIsCorrect())
                {
                    answerIsGet = true;
                    ui->lineEditStatus->setText("ОТВЕТ ПОЛУЧЕН");
                }
                if (int(ui->tabWidget->currentIndex())==1 && !timerFindDevice->isActive()) {
                    //text_out->setTextBackgroundColor(QColor(0,0,0));
                    //text_out->setTextColor(QColor(255,255,255));
                    char *zData = new char [int(b.getnDataByte())];
                    zData = b.getData();
                    union{
                        float f;
                        char ie[4];
                    }transf;
                    int fg = 0;
                    if (b.isLongFrame())
                    {
                        fg = 5;
                    }
                    else
                    {
                        fg = 1;
                    }
                    if(b.CrcIsCorrect())
                    {
                        item = new QStandardItem(QString("Ok"));
                        item->setBackground(Qt::green);
                        model->setItem(numberRow, 6, item);
                    }
                    else
                    {
                        item = new QStandardItem(QString("Bad"));
                        item->setBackground(Qt::red);
                        model->setItem(numberRow, 6, item);

                    }
                    QString ghb = QByteArray(b.getAddress(),fg).toHex();
                    QString gh;
                    if (b.isLongFrame()) gh = QString(ghb[4])+QString(ghb[5]);
                    else gh = ghb;
                    QString fileDir = QString("data/data_")+QString(gh)+QString(".txt");
                    QFile file1(fileDir);

                    QString fileDirCalibration;
                    if(ui->checkBoxCakibrationMiddle->checkState())
                    {
                        fileDirCalibration = QString("data/dataCalibrationMiddle_")+QString(gh)+QString(".txt");
                    }else if(ui->checkBoxCakibrationLow->checkState())
                    {
                       fileDirCalibration = QString("data/dataCalibrationLow_")+QString(gh)+QString(".txt");
                    }else if(ui->checkBoxCakibrationHigh->checkState())
                    {
                        fileDirCalibration = QString("data/dataCalibrationHigh_")+QString(gh)+QString(".txt");
                    }

                    QFile fileCalibration;
                    fileCalibration.setFileName(fileDirCalibration);
                    if(!file1.open(QIODevice::Append | QIODevice::Text))
                    {
                        item = new QStandardItem(QString("Ошибка открытия"));
                        item->setBackground(QColor(255,128,0));
                        model->setItem(numberRow, 7, item);

                    }
                    else
                    {
                        item = new QStandardItem(QString("Записан"));
                        item->setBackground(QColor(0,128,128));
                        model->setItem(numberRow, 7, item);
                    }
                    bool IsWriteCalibration;
                    IsWriteCalibration = ui->checkBoxCakibrationHigh->checkState() || ui->checkBoxCakibrationLow->checkState() ||ui->checkBoxCakibrationMiddle->checkState();
                    if(IsWriteCalibration)
                    {
                        if(!fileCalibration.open(QIODevice::Append | QIODevice::Text))
                        {
                            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
                            ui->lineEditStatus->setText("Файл калибровки не записан");
                        }else
                        {
                            ui->lineEditStatus->setText("Успешно");
                        }
                    }
                    QDateTime timeNow = QDateTime::currentDateTime();
                    item = new QStandardItem(QString(gh));
                    model->setItem(numberRow, 0, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray(timeNow.toString("dd.MM.yyyy hh:mm:ss").toUtf8())+QByteArray("\t"));
                    transf.ie[0] = zData[3];//запись тока
                    transf.ie[1] = zData[2];//запись тока
                    transf.ie[2] = zData[1];//запись тока
                    transf.ie[3] = zData[0];//запись тока
                    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
                    model->setItem(numberRow, 1, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    transf.ie[0] = zData[8];//запись первой перемнной
                    transf.ie[1] = zData[7];//запись первой перемнной
                    transf.ie[2] = zData[6];//запись первой перемнной
                    transf.ie[3] = zData[5];//запись первой перемнной
                    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
                    model->setItem(numberRow, 2, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    if(b.CrcIsCorrect() && IsWriteCalibration) fileCalibration.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    transf.ie[0] = zData[13];//запись второй перемнной
                    transf.ie[1] = zData[12];//запись второй перемнной
                    transf.ie[2] = zData[11];//запись второй перемнной
                    transf.ie[3] = zData[10];//запись второй перемнной
                    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
                    model->setItem(numberRow, 3, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    if(b.CrcIsCorrect() && IsWriteCalibration) fileCalibration.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    transf.ie[0] = zData[18];//запись третьей перемнной
                    transf.ie[1] = zData[17];//запись третьей перемнной
                    transf.ie[2] = zData[16];//запись третьей перемнной
                    transf.ie[3] = zData[15];//запись третьей перемнной
                    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
                    model->setItem(numberRow, 4, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    transf.ie[0] = zData[23];//запись четвертой перемнной
                    transf.ie[1] = zData[22];//запись четвертой перемнной
                    transf.ie[2] = zData[21];//запись четвертой перемнной
                    transf.ie[3] = zData[20];//запись четвертой перемнной
                    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
                    model->setItem(numberRow, 5, item);
                    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\n"));
                    if(b.CrcIsCorrect() && IsWriteCalibration) fileCalibration.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
                    ui->tableView->setModel(model);
                    ui->tableView->resizeRowsToContents();
                    ui->tableView->resizeColumnsToContents();
                    numberRow++;
//                    if(numberRow > 6)
//                    {
//                        model->removeRows(1,numberRow);
//                        numberRow = 1;
//                    }
                    file1.close();
                    fileCalibration.close();
                }
                if(!ui->checkAltView->checkState())
                {
                    //куда выводить
                    text_out->setTextBackgroundColor(QColor(0,0,0));
                    text_out->setTextColor(QColor(255,255,255));
                    //ui->textEdit->insertPlainText(outText+QString(""));
                    if(int(ui->tabWidget->currentIndex())==0)
                    {
                        if(!b.CrcIsCorrect()){
                            text_out->setTextBackgroundColor(QColor(255,0,0));
                            text_out->setTextColor(QColor(0,0,0));
                            text_out->insertPlainText("CRC Bad");
                            text_out->setTextBackgroundColor(QColor(255,255,255));
                            text_out->setTextColor(QColor(0,0,0));
                        }else
                        {
                            text_out->setTextBackgroundColor(QColor(0,255,0));
                            text_out->setTextColor(QColor(0,0,0));
                            text_out->insertPlainText("CRC Ok");
                            text_out->setTextBackgroundColor(QColor(255,255,255));

                        }
                    text_out->setTextColor(QColor(0,0,0));
                    }
                }
                else
                {
                    answer b(nSymAnsGet);
                    b.createAnswer(ansGet,nSymAnsGet);
                    b.analysis();
                    int n = 0;
                    char *zData = new char [int(b.getnDataByte())];
                    zData = b.getData();
                    char t;
                    ui->textEdit->clear();
                    for(n=0;n<b.getnDataByte();n++){
                        t = zData[n];
                        int ghjghj = n+(int)ReqData[0];
                        ui->textEdit->insertPlainText(QString("Address::")+QString("\t%1>:").number(ghjghj)+QString("\tData::")+QByteArray(1,t).toHex()+QString("\n"));
                        if(((n+1)%4 == 0) & (n!=0)){
                            union{
                                float f;
                                char ie[4];
                            }transf;
                            transf.ie[0] = zData[3+int((n-3))];
                            transf.ie[1] = zData[2+int((n-3))];
                            transf.ie[2] = zData[1+int((n-3))];
                            transf.ie[3] = zData[0+int((n-3))];
                            text_out->setTextBackgroundColor(QColor(0,0,0));
                            text_out->setTextColor(QColor(255,255,255));
                            text_out->insertPlainText(QString("Float ")+QString("\t%1>:").number(-3+int(n/1)+ReqData[0])+QString(" to ")+QString("\t%1>:").number(int(n/1)+ReqData[0])+QString(":\t")+QString("%1").number(transf.f)+QString("\n"));
                            text_out->setTextBackgroundColor(QColor(255,255,255));
                            text_out->setTextColor(QColor(0,0,0));
                        }
                    }

                }
                answerStart = false;
                nSymAnsGet = 0;
                prLength = 0;
            }
        }
        if(buf[i] != char(0xff) & prLength>=3 & !answerStart){
            answerStart = true;
            int j = 0;
            for(j = 0; j<prLength; j++){
                ansGet[nSymAnsGet] = char(0xff);
                nSymAnsGet++;
            }
            ansGet[nSymAnsGet] = buf[i];
            nSymAnsGet++;
        }else if((buf[i] == char(0xff)) & !answerStart){
            prLength++;
        }
    }
    QTextCursor sb = ui->textEdit->textCursor();
    sb.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(sb);
}
void MainWindow::clearText()//clear
{
    ui->textEdit->clear();
}
void MainWindow::showTime(){
    if(!(serial->isOpen())){
        ui->textEdit->setText("Not Open");
    }
    MainWindow::sendRequest();
}
void MainWindow::startLoop()//timerstart
{
    timer->start(static_cast<int>(ui->doubleSpinBoxTimerSend->value()*1000));
    if(ui->comboBoxFunc->currentData().toInt() == 91){
        ui->buttonSend->setEnabled(false);
        ui->buttonTimerStart->setEnabled(false);
        ui->pushButton_7->setEnabled(true);
    }
}
void MainWindow::stopLoop()//timer stop
{
    timer->stop();
}
void MainWindow::on_comboBoxFunc_currentIndexChanged()//отклик на изменение функции
{
    NumFunc = ui->comboBoxFunc->currentData().toInt();
    switch (NumFunc) {
        case 0:
            lastFrame = ui->checkLongFrame->checkState();
            ui->checkLongFrame->setCheckState(Qt::Unchecked);
            ui->checkLongFrame->setEnabled(false);
            ui->checkAltView->setEnabled(false);
            ui->checkAltView->setCheckState(Qt::Unchecked);
            ui->pushButton_7->setEnabled(false);
            break;
        case 3:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            if(lastFrame)
            {
                ui->checkLongFrame->setCheckState(Qt::Checked);
            }
            else
            {
                ui->checkLongFrame->setCheckState(Qt::Unchecked);
            }
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setEnabled(false);
            ui->checkAltView->setCheckState(Qt::Unchecked);
            break;
        case 36:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            lastFrame = ui->checkLongFrame->checkState();
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setEnabled(false);
            ui->checkAltView->setCheckState(Qt::Unchecked);
            break;
        case 37:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setEnabled(false);
            ui->checkAltView->setCheckState(Qt::Unchecked);
            break;
        case 43:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setEnabled(false);
            ui->checkAltView->setCheckState(Qt::Unchecked);
            break;
        case 51:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setEnabled(true);
            ui->checkAltView->setCheckState(Qt::CheckState::Checked);
            ui->buttonSend->setEnabled(true);
            ui->buttonTimerStart->setEnabled(true);
            ui->pushButton_7->setEnabled(false);
            break;
        case 91:
            //ui->checkLongFrame->setCheckState(Qt::Checked);
            ui->checkLongFrame->setEnabled(true);
            ui->checkAltView->setCheckState(Qt::CheckState::Unchecked);
            ui->checkAltView->setEnabled(false);
            ui->buttonSend->setEnabled(false);
            ui->buttonTimerStart->setEnabled(false);
            ui->pushButton_7->setEnabled(true);
            create91Request();
            break;
    }
    changedInByteExpected();
    getRequestAddr(ui->checkLongFrame->checkState());

}
void MainWindow::on_spinBox_valueChanged()//изменение длины преамбулы
{

    data91 = new unsigned char[int(ui->spinData91->value())+1];
    changedInByteExpected();
    //getRequestAddr();
    createRequestOut();
}
void MainWindow::on_checkLongFrame_stateChanged()//отклик на длину фрейма
{
    data91 = new unsigned char[int(ui->spinData91->value())+1];
    changedInByteExpected();
    getRequestAddr(ui->checkLongFrame->checkState());
}
void MainWindow::on_checkEnTextBrows_stateChanged(int arg1)//отклsючение текстовго редктора
{
    if(arg1==0){
        ui->textEdit->setEnabled(false);
        ui->buttonClear->setEnabled(false);
    }else{
        ui->textEdit->setEnabled(true);
        ui->buttonClear->setEnabled(true);
    }
}
void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)//ввод дданных
{
    if(arg1.length() == 5){
        QByteArray text =QByteArray(arg1.toLocal8Bit());
        text.remove(2,1);
        QByteArray hex = QByteArray::fromHex(text);
        ReqData[0] = char(hex[0]);
        ReqData[1] = char(hex[1]);

        switch (NumFunc) {
            case 0:
                inBytesExpected = int(ui->spinBox->value())+19+2*2*int(ui->checkLongFrame->checkState());
                break;
            case 3:
                inBytesExpected = int(ui->spinBox->value())+31+2*2*int(ui->checkLongFrame->checkState());
                break;
            case 51:
                inBytesExpected = int(ui->spinBox->value())+7+int(ReqData[1])+2*int(ui->checkLongFrame->checkState());
                break;
            case 91:
                inBytesExpected = int(ui->spinBox->value())+9+2*int(ui->checkLongFrame->checkState());
                break;
        }
        createRequestOut();
    }
}
void MainWindow::on_spinData91_valueChanged(int arg1)//разрешение ввода данных
{
    union{
        float f;
        char ie[4];
    }transf;
    QByteArray text;
    QByteArray hex;
    QWidget* centralWidget = ui->centralWidget;
    QLineEdit * lineFloat;
    QLineEdit * lineFloatByte;
    int n = 0;
    int m = 0;
    for(n = 1; n<=6;n++)
    {
        QString findObject = "lineFloat_"+QString().number(n);
        lineFloat = centralWidget->findChild<QLineEdit *>(findObject);
        if(arg1>=n*4)
        {
            lineFloat->setEnabled(true);
        }
        else
        {
            lineFloat->setEnabled(false);
        }
        for(m = 1; m<=4;m++)
        {
            QString findObject = "lineFloat_"+QString().number(n)+"_Byte_"+QString().number(m);
            lineFloatByte = centralWidget->findChild<QLineEdit *>(findObject);
            if(arg1>=(n-1)*4+m)
            {
                lineFloatByte->setEnabled(true);
            }
            else
            {
                lineFloatByte->setEnabled(false);
            }
            text = lineFloatByte->text().toLocal8Bit();
            hex = QByteArray::fromHex(text);
            transf.ie[4-m] =char(hex[0]);
        }
        lineFloat->setText(QString("%1").number(transf.f));
    }
}
void MainWindow::on_lineFloat_1_textChanged()
{
    float re = ui->lineFloat_1->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    QByteArray(transf.ie[0],1).toHex();
    ui->lineFloat_1_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_1_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_1_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_1_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::on_lineFloat_2_textChanged()
{
    float re = ui->lineFloat_2->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    QByteArray(transf.ie[0],1).toHex();
    ui->lineFloat_2_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_2_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_2_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_2_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::on_lineFloat_3_textChanged()
{
    float re = ui->lineFloat_3->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    ui->lineFloat_3_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_3_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_3_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_3_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::on_lineFloat_4_textChanged()
{
    float re = ui->lineFloat_4->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    QByteArray(transf.ie[0],1).toHex();
    ui->lineFloat_4_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_4_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_4_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_4_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::on_lineFloat_5_textChanged()
{
    float re = ui->lineFloat_5->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    QByteArray(transf.ie[0],1).toHex();
    ui->lineFloat_5_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_5_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_5_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_5_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::on_lineFloat_6_textChanged()
{
    float re = ui->lineFloat_6->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    QByteArray(transf.ie[0],1).toHex();
    ui->lineFloat_6_Byte_1->setText(QByteArray(1,transf.ie[3]).toHex());
    ui->lineFloat_6_Byte_2->setText(QByteArray(1,transf.ie[2]).toHex());
    ui->lineFloat_6_Byte_3->setText(QByteArray(1,transf.ie[1]).toHex());
    ui->lineFloat_6_Byte_4->setText(QByteArray(1,transf.ie[0]).toHex());
}
void MainWindow::create91Request()//создание 91 запроса
{
    ui->buttonSend->setEnabled(true);
    ui->buttonTimerStart->setEnabled(true);
    int i = 0;
    data91 = new unsigned char[int(ui->spinData91->value())+1];
    QByteArray text =QByteArray(ui->lineAddr91->text().toLocal8Bit());
    QByteArray hex = QByteArray::fromHex(text); data91[i] = hex[0];
    i++;
    int n = 0;
    int m = 0;
    QLineEdit *lineFloatByte;
    QWidget* centralWidget = ui->centralWidget;
    for(n = 1; n<=6;n++)
    {
        for(m = 1; m<=4;m++)
        {
            QString findObject = "lineFloat_"+QString().number(n)+"_Byte_"+QString().number(m);
            lineFloatByte = centralWidget->findChild<QLineEdit *>(findObject);
            if(lineFloatByte->isEnabled()){
                QByteArray text =QByteArray(lineFloatByte->text().toLocal8Bit());
                QByteArray hex = QByteArray::fromHex(text); data91[i] = hex[0];
                i++;
            }
        }
    }
    createRequestOut();
    //ui->pushButton_7->setEnabled(false);
}
void MainWindow::requestFunction3(QComboBox *f)
{
    unsigned char *currentAddr = new unsigned char [5];
    if(!ui->checkLongFrame->checkState())
    {
        QString textAddr = f->currentText();
        QByteArray text =QByteArray(textAddr.toLocal8Bit());
        QByteArray hex = QByteArray::fromHex(text);
        currentAddr[0] = char(hex[0]);
    }
    else
    {
        QString textAddr = f->currentText();
        QByteArray text =QByteArray(textAddr.toLocal8Bit());
        QByteArray hex = QByteArray::fromHex(text);
        currentAddr[0] = 0x0f;
        currentAddr[1] = 0x02;
        currentAddr[2] = char(hex[0]);
        currentAddr[3] = 0xfd;
        currentAddr[4] = 0xef;
    }
    request a;
    bool longFrame = ui->checkLongFrame->checkState();
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(currentAddr);
    a.function3();
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    inBytesExpected = int(ui->spinBox->value())+31+2*int(ui->checkLongFrame->checkState());
    sendRequest();
    countTimerFinc3 = 0;
}
void MainWindow::Function3Send()//send Function3
{
    connect(timerFunction3,&QTimer::timeout,this,&MainWindow::sendTimerRequest);
    //timerSendRequest = 150;
    if(ui->checkDevice_1->checkState()) requset1IsSend = true;
    if(ui->checkDevice_2->checkState()) requset2IsSend = true;
    if(ui->checkDevice_3->checkState()) requset3IsSend = true;
    if(ui->checkDevice_4->checkState()) requset4IsSend = true;
    if(ui->checkDevice_5->checkState()) requset5IsSend = true;
    answerIsGet = true;
    countTimerFinc3 = 0;
    allRequestIsSend = false;
    if(requset5IsSend || requset4IsSend || requset3IsSend || requset2IsSend || requset1IsSend)
    {
       timerFunction3->start(timerSendRequest);
    }
}
void MainWindow::sendTimerRequest()
{
    if(countTimerFinc3 >(3000/timerSendRequest))//timeout
    {
        answerIsGet = true;
    }
    if (ui->checkDevice_1->checkState() && answerIsGet && requset1IsSend)
    {
        requestFunction3(ui->comboBoxAddressFunc3_1);
        requset1IsSend = false;
        return void();
    }
    if (ui->checkDevice_2->checkState() && answerIsGet && requset2IsSend)
    {
        requestFunction3(ui->comboBoxAddressFunc3_2);
        requset2IsSend = false;
        return void();
    }
    if (ui->checkDevice_3->checkState() && answerIsGet && requset3IsSend)
    {
        requestFunction3(ui->comboBoxAddressFunc3_3);
        requset3IsSend = false;
        return void();
    }
    if (ui->checkDevice_4->checkState() && answerIsGet && requset4IsSend)
    {
        requestFunction3(ui->comboBoxAddressFunc3_4);
        requset4IsSend = false;
        return void();
    }
    if (ui->checkDevice_5->checkState() && answerIsGet && requset5IsSend)
    {
        requestFunction3(ui->comboBoxAddressFunc3_5);
        requset5IsSend = false;
        return void();
    }
    countTimerFinc3++;
    qDebug()<<countTimerFinc3;
    if(!requset5IsSend && !requset4IsSend && !requset3IsSend && !requset2IsSend && !requset1IsSend)
    {
       allRequestIsSend = true;
       countTimerFinc3 = 0;
       disconnect(timerFunction3,&QTimer::timeout,this,&MainWindow::sendTimerRequest);
       timerFunction3->stop();
       qDebug()<<"timerstop";
    }
//    timerFunction3->stop();
//    qDebug()<<"timerstop"<<endl;


    /*bool fsend = false;
    if(ui->checkDevice_1->checkState() && !fsend && requset1IsSend)
    {
        if(!ui->checkLongFrame->checkState())
        {
            QString textAddr = ui->comboBoxAddressFunc3_1->currentText();;
            if(textAddr.length() == 2){
                QByteArray text =QByteArray(textAddr.toLocal8Bit());
                text.remove(2,1);
                QByteArray hex = QByteArray::fromHex(text);
                ReqAddr[0] = char(hex[0]);
                createRequestOut();
            }
        }
        MainWindow::sendRequest();
        requset1IsSend = false;
        fsend = true;
    }
    if(ui->checkDevice_2->checkState() && !fsend && requset2IsSend)
    {
        if(!ui->checkLongFrame->checkState())
        {
            QString textAddr = ui->comboBoxAddressFunc3_2->currentText();;
            if(textAddr.length() == 2){
                QByteArray text =QByteArray(textAddr.toLocal8Bit());
                text.remove(2,1);
                QByteArray hex = QByteArray::fromHex(text);
                ReqAddr[0] = char(hex[0]);
                createRequestOut();
            }
        }
        MainWindow::sendRequest();
        requset2IsSend = false;
        fsend = true;
    }
    if(ui->checkDevice_3->checkState() && !fsend && requset3IsSend)
    {
        if(!ui->checkLongFrame->checkState())
        {
            QString textAddr = ui->comboBoxAddressFunc3_3->currentText();;
            if(textAddr.length() == 2){
                QByteArray text =QByteArray(textAddr.toLocal8Bit());
                text.remove(2,1);
                QByteArray hex = QByteArray::fromHex(text);
                ReqAddr[0] = char(hex[0]);
                createRequestOut();
            }
        }
        MainWindow::sendRequest();
        requset3IsSend = false;
        fsend = true;
    }
    if(ui->checkDevice_4->checkState() && !fsend && requset4IsSend)
    {
        if(!ui->checkLongFrame->checkState())
        {
            QString textAddr = ui->comboBoxAddressFunc3_4->currentText();;
            if(textAddr.length() == 2){
                QByteArray text =QByteArray(textAddr.toLocal8Bit());
                text.remove(2,1);
                QByteArray hex = QByteArray::fromHex(text);
                ReqAddr[0] = char(hex[0]);
                createRequestOut();
            }
        }
        MainWindow::sendRequest();
        requset4IsSend = false;
        fsend = true;
    }
    if(ui->checkDevice_5->checkState() && !fsend && requset5IsSend)
    {
        if(!ui->checkLongFrame->checkState())
        {
            QString textAddr = ui->comboBoxAddressFunc3_5->currentText();;
            if(textAddr.length() == 2){
                QByteArray text =QByteArray(textAddr.toLocal8Bit());
                text.remove(2,1);
                QByteArray hex = QByteArray::fromHex(text);
                ReqAddr[0] = char(hex[0]);
                createRequestOut();
            }
        }
        MainWindow::sendRequest();
        requset5IsSend = false;
        fsend = true;
    }
    requestIsSend++;
    //qDebug()<<requestIsSend;
    if(requestIsSend == pLo)
    {
        timerFunction3->stop();
        requestIsSend = 0;
    }*/
}
void MainWindow::sendTimerRequestLoop()
{
    if(allRequestIsSend)
    {
        Function3Send();
    }
}
void MainWindow::startLoopFunction3()
{    
    connect(timerFunction3Loop,&QTimer::timeout,this,&MainWindow::sendTimerRequestLoop);
    Function3Send();
    timerFunction3Loop->start(timerSendRequest);
}
void MainWindow::stopLoopFunction3()
{
    disconnect(timerFunction3Loop,&QTimer::timeout,this,&MainWindow::sendTimerRequestLoop);
    timerFunction3Loop->stop();
}
void MainWindow::clearTable()
{
    model->clear();
    numberRow = 0;
}
void MainWindow::on_tabWidget_currentChanged(int index)
{
    changedInByteExpected();
    switch (index) {
        case 0:        
            ui->comboBoxFunc->setCurrentIndex(lastFunc);
            //ui->comboBoxFunc->setCurrentIndex(0);
            getRequestAddr(ui->checkLongFrame->checkState());
            break;
        case 1:
            lastFunc = ui->comboBoxFunc->currentIndex();
            ui->comboBoxFunc->setCurrentIndex(1); break;
    }
}
void MainWindow::on_lineAddrShort_textChanged()
{
    getRequestAddr(ui->checkLongFrame->checkState());
}
void MainWindow::on_lineAddrLong_textChanged()
{
    getRequestAddr(ui->checkLongFrame->checkState());
}

void MainWindow::on_checkDevice_1_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_1->currentText(), ui->checkDevice_1->checkState());
}
void MainWindow::on_checkDevice_2_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_2->currentText(), ui->checkDevice_2->checkState());
}
void MainWindow::on_checkDevice_3_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_3->currentText(), ui->checkDevice_3->checkState());
}
void MainWindow::on_checkDevice_4_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_4->currentText(), ui->checkDevice_4->checkState());
}
void MainWindow::on_checkDevice_5_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_5->currentText(), ui->checkDevice_5->checkState());
}
void MainWindow::findDevice()//запуск таймера на поиск
{

    ui->comboBoxFunc->setCurrentIndex(0);
    ReqAddr[0] = 0x00;
    ui->comboBoxAddress->clear();
    ui->comboBoxAddressFunc3_1->clear();
    ui->comboBoxAddressFunc3_2->clear();
    ui->comboBoxAddressFunc3_3->clear();
    ui->comboBoxAddressFunc3_4->clear();
    ui->comboBoxAddressFunc3_5->clear();
    ui->buttonSend->setEnabled(false);
    ui->buttonFunction3Send->setEnabled(false);
    ui->buttonTimerStart->setEnabled(false);
    ui->buttonFunction3Loop->setEnabled(false);
    ui->comboBoxAddress->setEnabled(false);
    ui->comboBoxAddressFunc3_1->setEnabled(false);
    ui->comboBoxAddressFunc3_2->setEnabled(false);
    ui->comboBoxAddressFunc3_3->setEnabled(false);
    ui->comboBoxAddressFunc3_4->setEnabled(false);
    ui->comboBoxAddressFunc3_5->setEnabled(false);
    timerFindDevice->start(400);

    request a;
    a.setLongFrame(0);
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(ReqAddr);
    a.function0();
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    sendRequest();
    //inBytesExpected = int(ui->spinBox->value())+19+2*int(ui->checkLongFrame->checkState());
}
void MainWindow::sendFindRequest()//создание запроса на поиск
{
    ui->comboBoxFunc->setCurrentIndex(0);
    if(answerIsGet)
    {
        QByteArray outText2 = QByteArray(1,ReqAddr[0]).toHex();
        ui->comboBoxAddress->addItem(QString(outText2));
        ui->comboBoxAddressFunc3_1->addItem(QString(outText2));
        ui->comboBoxAddressFunc3_2->addItem(QString(outText2));
        ui->comboBoxAddressFunc3_3->addItem(QString(outText2));
        ui->comboBoxAddressFunc3_4->addItem(QString(outText2));
        ui->comboBoxAddressFunc3_5->addItem(QString(outText2));
    }
    ReqAddr[0]++;
    if(ReqAddr[0] == 0x10)
    {
        timerFindDevice->stop();
        ui->comboBoxFunc->setCurrentIndex(1);
        ui->buttonSend->setEnabled(true);
        ui->buttonFunction3Send->setEnabled(true);
        ui->buttonTimerStart->setEnabled(true);
        ui->buttonFunction3Loop->setEnabled(true);
        ui->comboBoxAddress->setEnabled(true);
        ui->comboBoxAddressFunc3_1->setEnabled(true);
        ui->comboBoxAddressFunc3_2->setEnabled(true);
        ui->comboBoxAddressFunc3_3->setEnabled(true);
        ui->comboBoxAddressFunc3_4->setEnabled(true);
        ui->comboBoxAddressFunc3_5->setEnabled(true);
        ui->textEdit->clear();
        getRequestAddr(ui->checkLongFrame->checkState());
    }else
    {
        request a;
        a.setLongFrame(0);
        a.setPreambleLength(ui->spinBox->value());
        a.setAddress(ReqAddr);
        a.function0();
        answerIsGet = false;
        QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
        int i = 0;
        for(i = 2; i<req.length();i+=3){
            req.insert(i,' ');
        }
        ui->lineRequest->setText(req);
        sendRequest();
    }
}
void MainWindow::on_comboBoxAddress_highlighted()
{
        getRequestAddr(ui->checkLongFrame->checkState());
        ui->comboBoxAddressFunc3_1->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_2->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_3->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_4->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
}
void MainWindow::on_comboBoxAddress_currentIndexChanged()
{
        getRequestAddr(ui->checkLongFrame->checkState());
        ui->comboBoxAddressFunc3_1->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_2->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_3->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_4->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
}

void MainWindow::spanRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationSpan);
    request a;
    bool longFrame = ui->checkLongFrame->checkState();
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(ReqAddr);
    a.function36();
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    ui->label_3->setStyleSheet(styleSheetCalibrationBad);
    inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
    timerCalibration->start(50);
    sendRequest();
}
void MainWindow::indicateCalibrationSpan()
{

    if(answerIsGet)
    {
        ui->label_3->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationSpan);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_3->setStyleSheet(styleSheetCalibrationDefault);        
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationSpan);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::zeroRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZero);
    request a;
    bool longFrame = ui->checkLongFrame->checkState();
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(ReqAddr);
    a.function37();
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    ui->label_4->setStyleSheet(styleSheetCalibrationBad);
    inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
    timerCalibration->start(50);
    sendRequest();
}
void MainWindow::indicateCalibrationZero()
{

    if(answerIsGet)
    {
        ui->label_4->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZero);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_4->setStyleSheet(styleSheetCalibrationDefault);        
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZero);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::zeroFirstVarRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZeroFirstVar);
    request a;
    bool longFrame = ui->checkLongFrame->checkState();
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    a.setAddress(ReqAddr);
    a.function43();
    answerIsGet = false;
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    ui->label_5->setStyleSheet(styleSheetCalibrationBad);
    inBytesExpected = int(ui->spinBox->value())+7+2*int(ui->checkLongFrame->checkState());
    timerCalibration->start(50);
    sendRequest();
}
void MainWindow::indicateCalibrationZeroFirstVar()
{

    if(answerIsGet)
    {
        ui->label_5->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZeroFirstVar);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_5->setStyleSheet(styleSheetCalibrationBad);        
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateCalibrationZeroFirstVar);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::checkPassword()
{
    QString gh = ui->linePassword->text();
    if (gh =="1234")
    {
        ui->tabWidget->setCurrentIndex(0);
        //ui->tabWidget->setTabEnabled(0,true);
    }else
    {
        ui->tabWidget->setCurrentIndex(1);
        //ui->tabWidget->setTabEnabled(0,false);
    }
}

void MainWindow::calibrationFunctions(unsigned char *data,int numberData)
{
    request a;
    bool longFrame = true;
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    getRequestAddr(true);
    a.setAddress(ReqAddr);
    a.function91(data,numberData);
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    //inBytesExpected = int(ui->spinBox->value())+9+2*int(ui->checkLongFrame->checkState());
    inBytesExpected = int(ui->spinBox->value())+13;
    sendRequest();
}
void MainWindow::calibrationFunctionsGet(unsigned char *data1,int numberData1)
{
    request a;
    bool longFrame = false;
    getRequestAddr(longFrame);
    a.setLongFrame(longFrame);
    a.setPreambleLength(ui->spinBox->value());
    getRequestAddr(false);
    a.setAddress(ReqAddr);
    a.function51(data1);
    QByteArray req = QByteArray(a.getRequest(),a.getRequestLength()).toHex();
    int i = 0;
    for(i = 2; i<req.length();i+=3){
        req.insert(i,' ');
    }
    ui->lineRequest->setText(req);
    //inBytesExpected = int(ui->spinBox->value())+numberData1+6+2*int(ui->checkLongFrame->checkState());
    inBytesExpected = int(ui->spinBox->value())+numberData1+6;
    sendRequest();
}

void MainWindow::setAddress()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetAddress);
    unsigned char *dataChangeAddress = new unsigned char [2];
    QString textAddr = ui->comboBoxSetAddress->currentText();
    QByteArray text =QByteArray(textAddr.toLocal8Bit());
    QByteArray hex = QByteArray::fromHex(text);
    dataChangeAddress[0] = 0x03;
    dataChangeAddress[1] =(unsigned char)hex[0];
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void MainWindow::setMode()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMode);
    unsigned char *dataChangeAddress = new unsigned char [2];
    int textMode = ui->comboBoxSetMode->currentData().toInt();
    dataChangeAddress[0] = 0x01;
    dataChangeAddress[1] =(unsigned char)textMode;
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void MainWindow::setMaxValue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue);
    float re = ui->lineEditSetMaxValue->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    unsigned char *maxValue = new unsigned char [5];
    maxValue[0] = 0x1e;
    maxValue[1] = transf.ie[3];
    maxValue[2] = transf.ie[2];
    maxValue[3] = transf.ie[1];
    maxValue[4] = transf.ie[0];
    timerCalibration->start(50);
    calibrationFunctions(maxValue,5);
}
void MainWindow::setMaxValue_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue_2);
    float re = ui->lineEditSetMaxValue_2->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    unsigned char *maxValue = new unsigned char [5];
    maxValue[0] = 0x22;
    maxValue[1] = transf.ie[3];
    maxValue[2] = transf.ie[2];
    maxValue[3] = transf.ie[1];
    maxValue[4] = transf.ie[0];
    timerCalibration->start(50);
    calibrationFunctions(maxValue,5);
}
void MainWindow::setMovingAverage_1()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_1);
    unsigned char *dataChangeAddress = new unsigned char [2];
    dataChangeAddress[0] = 0x0b;
    dataChangeAddress[1] = (unsigned char)ui->spinBoxMovingAverage_1->value();
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void MainWindow::setMovingAverage_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_2);
    unsigned char *dataChangeAddress = new unsigned char [2];
    dataChangeAddress[0] = 0x0d;
    dataChangeAddress[1] = (unsigned char)ui->spinBoxMovingAverage_2->value();
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void MainWindow::setA_40()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_40);
    float re = ui->lineEditSetA_40->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    unsigned char *maxValue = new unsigned char [5];
    maxValue[0] = 0x4e;
    maxValue[1] = transf.ie[3];
    maxValue[2] = transf.ie[2];
    maxValue[3] = transf.ie[1];
    maxValue[4] = transf.ie[0];
    timerCalibration->start(50);
    calibrationFunctions(maxValue,5);
}
void MainWindow::setA_41()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_41);
    float re = ui->lineEditSetA_41->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    unsigned char *maxValue = new unsigned char [5];
    maxValue[0] = 0x52;
    maxValue[1] = transf.ie[3];
    maxValue[2] = transf.ie[2];
    maxValue[3] = transf.ie[1];
    maxValue[4] = transf.ie[0];
    timerCalibration->start(50);
    calibrationFunctions(maxValue,5);
}
void MainWindow::setA_42()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_42);
    float re = ui->lineEditSetA_42->text().toFloat();
    union{
        float f;
        char ie[4];

    }transf;
    transf.f = re;
    unsigned char *maxValue = new unsigned char [5];
    maxValue[0] = 0x56;
    maxValue[1] = transf.ie[3];
    maxValue[2] = transf.ie[2];
    maxValue[3] = transf.ie[1];
    maxValue[4] = transf.ie[0];
    timerCalibration->start(50);
    calibrationFunctions(maxValue,5);
}

void MainWindow::getAddress()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetAddress);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x03;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void MainWindow::getMode()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMode);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x01;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void MainWindow::getMaxValue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x1e;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getMaxValue_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue_2);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x22;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getMovingAverage_1()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_1);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x0b;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void MainWindow::getMovingAverage_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_2);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x0d;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void MainWindow::getA_40()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_40);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x4e;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getA_41()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_41);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x52;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getA_42()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_42);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x56;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getPressue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetPressue);
    unsigned char *data = new unsigned char [2];
    data[0] = 0xcc;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void MainWindow::getCurrent()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetCurrent);
    unsigned char *data = new unsigned char [2];
    data[0] = 0xc8;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}

void MainWindow::indicateSetAddress()
{
    if(answerIsGet)
    {
        ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetAddress);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetAddress);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetMode()
{
    if(answerIsGet)
    {
        ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMode);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMode);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetMaxValue()
{
    if(answerIsGet)
    {
        ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetMaxValue_2()
{
    if(answerIsGet)
    {
        ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMaxValue_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetMovingAverage_1()
{
    if(answerIsGet)
    {
        ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_1);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_1);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetMovingAverage_2()
{
    if(answerIsGet)
    {
        ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetMovingAverage_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetA_40()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_40);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_40);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetA_41()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_41);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_41);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateSetA_42()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_42);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetA_42);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::indicateGetAddress()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        char f =zData[0];
        QByteArray outText = QByteArray(1,f).toHex();
        ui->lineEditGetAddress->setText(outText);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetAddress);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetAddress->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetAddress);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetMode()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        char f =zData[0];
        if(f == 0x02)
        {
            ui->lineEditGetMode->setText("HART");

        }else if(f == 0x04)
        {
            ui->lineEditGetMode->setText("RS-485");
        }
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMode);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMode->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMode);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetMaxValue()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditMaxValueGet->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;

        ui->lineEditMaxValueGet->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetMaxValue_2()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditMaxValueGet_2->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;

        ui->lineEditMaxValueGet_2->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMaxValue_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetMovingAverage_1()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        int f =zData[0];
        ui->lineEditGetMovingAverage_1->setText(QString().number(f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_1);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMovingAverage_1->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_1);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetMovingAverage_2()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        int f =zData[0];
        ui->lineEditGetMovingAverage_2->setText(QString().number(f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMovingAverage_2->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetMovingAverage_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetA_40()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditGetA_40->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_40);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_40->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_40);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetA_41()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditGetA_41->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_41);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_41->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_41);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetA_42()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditGetA_42->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_42);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_42->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetA_42);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetPressue()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditGetPressue->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetPressue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetPressue->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetPressue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void MainWindow::indicateGetCurrent()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(inBytesExpected);
        b.createAnswer(ansGet,inBytesExpected);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        union{
            float f;
            char ie[4];

        }transf;
        transf.ie[0] = zData[3];
        transf.ie[1] = zData[2];
        transf.ie[2] = zData[1];
        transf.ie[3] = zData[0];
        ui->lineEditGetCurrent->setText(QString().number(transf.f));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetCurrent);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetCurrent->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateGetCurrent);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::on_checkBoxFixedCurrent_stateChanged()
{
    //ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    ui->buttonSetFixedCurrent->setEnabled(false);
    ui->lineEditSetFixedCurrent->setEnabled(false);
    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateStateFixedCurren);
    unsigned char *dataChangeAddress = new unsigned char [2];
    dataChangeAddress[0] = 0x19;
    if(ui->checkBoxFixedCurrent->checkState())
    {
        dataChangeAddress[1] = 0x01;
    }else
    {
        dataChangeAddress[1] = 0x00;
    }
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void MainWindow::indicateStateFixedCurren()
{
    if(answerIsGet)
    {
        //ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        ui->buttonSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        ui->lineEditSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateStateFixedCurren);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->buttonSetFixedCurrent->setEnabled(false);
        ui->lineEditSetFixedCurrent->setEnabled(false);
        //ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateStateFixedCurren);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::setValueFixedCurrent()
{

    connect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetValueFixedCurrent);
    double text = ui->lineEditSetFixedCurrent->text().toDouble();
    if(text<4) text = 4;
    if(text>20) text = 20;
    int c =(text-4)*65535/16;
    int a = c/256;
    int b = c%256;
    //qDebug()<<a;
    //qDebug()<<b;
    unsigned char *value = new unsigned char [3];
    value[0] = 0x1a;
    value[1] = (unsigned char)a;
    value[2] = (unsigned char)b;
    timerCalibration->start(50);
    calibrationFunctions(value,3);
}
void MainWindow::indicateSetValueFixedCurrent()
{
    if(answerIsGet)
    {
        ui->lineEditSetFixedCurrent->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetValueFixedCurrent);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetFixedCurrent->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &MainWindow::indicateSetValueFixedCurrent);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void MainWindow::on_checkBoxCakibrationMiddle_stateChanged()
{
    if(ui->checkBoxCakibrationMiddle->checkState())
    {
        ui->checkBoxCakibrationHigh->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationLow->setCheckState(Qt::Unchecked);
    }
}
void MainWindow::on_checkBoxCakibrationLow_stateChanged()
{
    if(ui->checkBoxCakibrationLow->checkState())
    {
        ui->checkBoxCakibrationHigh->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationMiddle->setCheckState(Qt::Unchecked);
    }
}
void MainWindow::on_checkBoxCakibrationHigh_stateChanged()
{
    if(ui->checkBoxCakibrationHigh->checkState())
    {
        ui->checkBoxCakibrationLow->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationMiddle->setCheckState(Qt::Unchecked);
    }
}
void MainWindow::findCoef()
{
    QString fileDirCalibration;
    fileDirCalibration = QString("data/dataCalibrationLow_")+ui->comboBoxAddress->currentText()+QString(".txt");
    QFile fileCalibration(fileDirCalibration);
    if(!fileCalibration.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(102,0,192);color:white}");
        ui->lineEditStatus->setText("Файл калибровки не открыт или не существует");
    }else
    {
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(143,252,172);color:rgb(3,0,87)}");
        ui->lineEditStatus->setText("Файл калибровки Открыт");
    }
    QString hLow(fileCalibration.readAll());
    fileCalibration.close();
    float *ULow = new float[3];
    ULow = findMx(hLow);

    fileDirCalibration = QString("data/dataCalibrationMiddle_")+ui->comboBoxAddress->currentText()+QString(".txt");
    fileCalibration.setFileName(fileDirCalibration);
    if(!fileCalibration.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->lineEditStatus->setText("Файл калибровки не открыт или не существует");
    }else
    {
        ui->lineEditStatus->setText("Файл калибровки Открыт");
    }
    QString hMidle(fileCalibration.readAll());
    fileCalibration.close();
    float *UMiddle = new float[3];
    UMiddle = findMx(hMidle);

    fileDirCalibration = QString("data/dataCalibrationHigh_")+ui->comboBoxAddress->currentText()+QString(".txt");
    fileCalibration.setFileName(fileDirCalibration);
    if(!fileCalibration.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->lineEditStatus->setText("Файл калибровки не открыт или не существует");
    }else
    {
        ui->lineEditStatus->setText("Файл калибровки Открыт");
    }
    QString hHigh(fileCalibration.readAll());
    fileCalibration.close();
    float *UHigh = new float[3];
    UHigh = findMx(hHigh);
    float U4n = UMiddle[2];
    float U4l = ULow[2];
    float U4h = UHigh[2];
    float U1l = ULow[0];
    float U1h = UHigh[0];
    float U2l = ULow[1];
    float U2h = UHigh[1];

    float U10 = ui->lineEditMaxValueGet_2->text().toFloat();
    float U20 = ui->lineEditMaxValueGet_2->text().toFloat();
    if(ui->lineEditMaxValueGet_2->text() == "nan" || ui->lineEditMaxValueGet_2->text() == "Bad Crc" || ui->lineEditMaxValueGet_2->text() == "")
    {
        ui->lineEditStatus->setText("Не получено максимальное значение верхнего диапазона измерения перепада давления");
        return void();
    }
    float Umax = ui->lineEditMaxValueGet->text().toFloat();
    if(ui->lineEditMaxValueGet->text() == "nan" || ui->lineEditMaxValueGet->text() == "Bad Crc"  || ui->lineEditMaxValueGet->text() == "")
    {
        ui->lineEditStatus->setText("Не получено максимальное значение статического давления");
        return void();
    }
    float K1 = Umax/(U1h-U10);
    float K2 = Umax/(U2h-U20);
    float a42 = 0,a41 = 0,a40 = 0;
    a42 = -(K1*U10*U4h - K2*U20*U4h - K1*U10*U4l + K2*U20*U4l + K1*U1h*U4l - K1*U4h*U1l - K2*U2h*U4l + K2*U4h*U2l - K1*U1h*U4n + K2*U2h*U4n + K1*U1l*U4n - K2*U2l*U4n)/((U4h - U4l)*(U4h - U4n)*(U4l - U4n));
    a41 = (- a42*U4l*U4l + a42*U4n*U4n + K1*(U10 - U1l) - K2*(U20 - U2l))/(U4l - U4n);
    a40 = - a42*U4n*U4n - a41*U4n;
    ui->lineEditSetA_42->setText(QString().number(a42));
    ui->lineEditSetA_41->setText(QString().number(a41));
    ui->lineEditSetA_40->setText(QString().number(a40));

}
void MainWindow::clearCalibrationData()
{
    removeFile(ui->comboBoxAddress->currentText());
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    timerSendRequest = arg1;
}
