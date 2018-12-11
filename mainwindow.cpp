#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "request.h"
#include "answer.h"
#include "calibration.h"
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
//static answer b(nSymHartAnswer);
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
static bool requset1FindCoef = 0;
static bool requset2FindCoef = 0;
static bool requset3FindCoef = 0;
static bool requset4FindCoef = 0;
static bool requset5FindCoef = 0;
static bool answerAnalisys = 0;
static float Pkstat = 0;
static float Uk1h = 0;
static float Uk2h = 0;
static float Uk10 = 0;
static float Uk20 = 0;
static float U4n = 0;
static float U4l = 0;
static float U4h = 0;
static float U1l = 0;
static float U1h = 0;
static float U2l = 0;
static float U2h = 0;
static char *ansGet_m = new char[50];
static bool IsPreamble = true;
static int nSymPreamble = 0;
static int nSymHartAnswer = 0;
static bool LongFrame = false;
static int nSymDataHartAnswer = 0;
static int nSymHartAnswerTotal = 0;
static float PPos = 100;
static float PNeg = 100;
static float avarageU1 = 0;
static float avarageU2 = 0;
static float avarageU3 = 0;
static float avarageU4 = 0;
static int Navarage = 0;
static float dAvarageU1 [5][4];
static int dNavarage [5];

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
HartTester::HartTester(QWidget *parent) :
    QMainWindow(parent),
    setDialog(new Dialog),
    ui(new Ui::HartTester),
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

        PPos = ui->spinBoxPressuePos->value();
        PNeg = ui->spinBoxPressueNeg->value();

        for(int i = 0; i<5;i++)
        {
            for(int j = 0; j<4;j++)
            {
                dAvarageU1[i][j] = 0;
            }
            dNavarage[i] = 0;
        }
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
        ui->actionDialog->setEnabled(false);


    }
HartTester::~HartTester()
{
    saveSettings();
    serial->close();
    delete ui;
}
void HartTester::connections()
{

    connect(ui->actionDialog,            &QAction::triggered,  &setDialog,&Dialog::show);

    connect(serial,                     &QSerialPort::readyRead,this,   &HartTester::readData_m);             //slot recieve data from COM port
    connect(timer,                      &QTimer::timeout,       this,   &HartTester::showTime);             //slot timer for sending loop request
    connect(timerFindDevice,            &QTimer::timeout,       this,   &HartTester::sendFindRequest);      //slot timer for sending find request
    connect(ui->buttonResetAddr,        &QPushButton::clicked,  this,   &HartTester::ResetAddressInit);     //slot button for reset combobox address
    connect(ui->pushButton,             &QPushButton::clicked,  this,   &HartTester::connectCOM);           //slot button for connection COM port
    connect(ui->buttonClose,            &QPushButton::clicked,  this,   &HartTester::closeCOM);             //slot button for closing COM port
    connect(ui->buttonSend,             &QPushButton::clicked,  this,   &HartTester::sendRequest);          //slot button for closing COM port
    connect(ui->buttonClear,            &QPushButton::clicked,  this,   &HartTester::clearText);            //slot button for clear developer TextEEdit
    connect(ui->buttonTimerStart,       &QPushButton::clicked,  this,   &HartTester::startLoop);            //slot button start timer sending loop request
    connect(ui->buttonTimerStop,        &QPushButton::clicked,  this,   &HartTester::stopLoop);             //slot button stop timer sending loop request
    connect(ui->pushButton_7,           &QPushButton::clicked,  this,   &HartTester::create91Request);      //slot button create request drom data for request91
    connect(ui->pushButton_10,          &QPushButton::clicked,  this,   &HartTester::stopLoopFunction3);    //slot button start timer for sending loop request for tab Function3
    connect(ui->pushButton_11,          &QPushButton::clicked,  this,   &HartTester::clearTable);           //slot button clear table for tab Function 3
    connect(ui->buttonFindDevice,       &QPushButton::clicked,  this,   &HartTester::findDevice);           //slot button start timer for sending find request
    connect(ui->buttonFunction3Send,    &QPushButton::clicked,  this,   &HartTester::Function3Send);        //slot button for sending request for tab Function3
    connect(ui->buttonFunction3Loop,    &QPushButton::clicked,  this,   &HartTester::startLoopFunction3);   //slot button start timer for sending loop request for tab Function3
    connect(ui->buttonSpan,             &QPushButton::clicked,  this,   &HartTester::spanRequest);          //slot button for sending Span request for tab Calibration
    connect(ui->buttonZero,             &QPushButton::clicked,  this,   &HartTester::zeroRequest);          //slot button for sending Zero request for tab Calibration
    connect(ui->buttonZeroFirstVar,     &QPushButton::clicked,  this,   &HartTester::zeroFirstVarRequest);  //slot button for sending request zero first variable for tab Calibration
    connect(ui->linePassword,           &QLineEdit::textEdited, this,   &HartTester::checkPassword);
    connect(ui->buttonSetAddress,       &QPushButton::clicked,  this,   &HartTester::setAddress);
    connect(ui->buttonSetMode,          &QPushButton::clicked,  this,   &HartTester::setMode);
    connect(ui->buttonSetMaxValue,      &QPushButton::clicked,  this,   &HartTester::setMaxValue);
    connect(ui->buttonSetMaxValue_2,    &QPushButton::clicked,  this,   &HartTester::setMaxValue_2);
    connect(ui->buttonMovingAverage_1,  &QPushButton::clicked,  this,   &HartTester::setMovingAverage_1);
    connect(ui->buttonMovingAverage_2,  &QPushButton::clicked,  this,   &HartTester::setMovingAverage_2);
    connect(ui->buttonSetA_40,          &QPushButton::clicked,  this,   &HartTester::setA_40);
    connect(ui->buttonSetA_41,          &QPushButton::clicked,  this,   &HartTester::setA_41);
    connect(ui->buttonSetA_42,          &QPushButton::clicked,  this,   &HartTester::setA_42);
    connect(ui->buttonGetAddress,       &QPushButton::clicked,  this,   &HartTester::getAddress);
    connect(ui->buttonGetMode,          &QPushButton::clicked,  this,   &HartTester::getMode);
    connect(ui->buttonGetMaxValue,      &QPushButton::clicked,  this,   &HartTester::getMaxValue);
    connect(ui->buttonGetMaxValue_2,    &QPushButton::clicked,  this,   &HartTester::getMaxValue_2);
    connect(ui->buttonGetMovingAverage_1,&QPushButton::clicked, this,   &HartTester::getMovingAverage_1);
    connect(ui->buttonGetMovingAverage_2,&QPushButton::clicked, this,   &HartTester::getMovingAverage_2);
    connect(ui->buttonGetA_40,          &QPushButton::clicked,  this,   &HartTester::getA_40);
    connect(ui->buttonGetA_41,          &QPushButton::clicked,  this,   &HartTester::getA_41);
    connect(ui->buttonGetA_42,          &QPushButton::clicked,  this,   &HartTester::getA_42);
    connect(ui->buttonSetFixedCurrent,  &QPushButton::clicked,  this,   &HartTester::setValueFixedCurrent);
    connect(ui->buttonGetCurrent,       &QPushButton::clicked,  this,   &HartTester::getCurrent);
    connect(ui->buttonGetPressue,       &QPushButton::clicked,  this,   &HartTester::getPressue);
    connect(ui->buttonUpdateCom,        &QPushButton::clicked,  this,   &HartTester::updateCom);
    connect(ui->buttonFindCoef,         &QPushButton::clicked,  this,   &HartTester::findCoef);
    connect(ui->buttonClearCalibration, &QPushButton::clicked,  this,   &HartTester::clearCalibrationData);
    connect(ui->buttonSetPressue,       &QPushButton::clicked,  this,   &HartTester::setPressue);
    connect(ui->buttonMO,               &QPushButton::clicked,  this,   &HartTester::MO);
    connect(ui->buttonClearMO,          &QPushButton::clicked,  this,   &HartTester::clearMO);
    connect(ui->buttonAddDataCoef,      &QPushButton::clicked,  this,   &HartTester::addDataCoef);
    connect(ui->buttonFindCoefMNK,      &QPushButton::clicked,  this,   &HartTester::findCoefMNK);
    connect(&setDialog, SIGNAL(MySetValueSignal(float*)), this, SLOT(testFunc(float*)));
}
void HartTester::downloadSettings()
{
    QSettings settings("STLab","HartTester");
    ui->tabWidget->setCurrentIndex      (settings.value("/Parameters/CurrentTab").toInt());
    ui->comboBoxAddress->setCurrentIndex(settings.value("/Parameters/CurrentAddr").toInt());
    ui->comboBoxFunc->setCurrentIndex   (settings.value("/Parameters/CurrentFunc").toInt());
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
    ui->spinBox_2->setValue(            settings.value("/Parameters/Frequence").toInt());
}
void HartTester::saveSettings()
{
    QSettings settings("STLab","HartTester");
    settings.setValue("/Parameters/CurrentAddr",    ui->comboBoxAddress->currentIndex());
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
    settings.setValue("/Parameters/Frequence",      ui->spinBox_2->value());
    //settings.setValue("/Parameters/SetA41",         ui->lineEditSetA_41->text().toFloat());
}
void HartTester::ResetAddressInit()
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
void HartTester::createRequestOut(bool tr = false)
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
void HartTester::getRequestAddr(bool b)
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
void HartTester::changedInByteExpected(){
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
void HartTester::showHideTableRow(QString r, bool t){
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
void HartTester::updateCom()
{
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
                ui->comboBox->addItem(info.portName());
//    QObject *g= sender();
//    if(ui->checkLongFrame->checkState())
//    {
//        ui->lineEditStatus->setText(g->metaObject()->className());
//    }

}
void HartTester::aboutHartTester()
{
    QMessageBox::about(this, "О программе", "HartTester Program \nVer. 0.20181101a");
}
void HartTester::aboutQt()
{
    QMessageBox::aboutQt(this);
}
void HartTester::connectCOM()//connect
{
    if (serial->portName() != ui->comboBox->currentText() || !serial->isOpen())
    {
          serial->close();
          serial->setPortName(ui->comboBox->currentText());
          serial->open(QSerialPort::ReadWrite);
          serial->setBaudRate(QSerialPort::Baud1200);
          serial->setDataBits(QSerialPort::Data8);
          serial->setParity(QSerialPort::OddParity);
          serial->setStopBits(QSerialPort::OneStop);
          serial->setFlowControl(QSerialPort::NoFlowControl);
    }
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
void HartTester::closeCOM()//close
{
    if(serial->isOpen()){
        ui->pushButton->setEnabled(true);
        ui->buttonClose->setEnabled(false);
    }
    timer->stop();
    serial->close();
}
void HartTester::sendRequest()//send
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

    nSymHartAnswer = 0;
    nSymHartAnswerTotal = 0;
    nSymHartAnswer = 0;
    nSymDataHartAnswer= 0;
    nSymPreamble = 0;
    IsPreamble = true;

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
void HartTester::displayData(answer b)
{
    QStandardItem *item;
    QStringList horizontalHeader;
    horizontalHeader.append("Адрес");
    horizontalHeader.append("Ток, мА");
    horizontalHeader.append("U1, мВ");
    horizontalHeader.append("U2, мВ");
    horizontalHeader.append("U3, мВ");
    horizontalHeader.append("U4, мВ");
    horizontalHeader.append("P+, кПа");
    horizontalHeader.append("P-, кПа");
    horizontalHeader.append("CRC");
    horizontalHeader.append("Файл");
    model->setHorizontalHeaderLabels(horizontalHeader);

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
        model->setItem(numberRow, 8, item);
    }
    else
    {
        item = new QStandardItem(QString("Bad"));
        item->setBackground(Qt::red);
        model->setItem(numberRow, 8, item);

    }
    QString ghb = QByteArray(b.getAddress(),fg).toHex();
    QString gh;
    if (b.isLongFrame()) gh = QString(ghb[4])+QString(ghb[5]);
    else gh = ghb;
    /*///////////////////////////////////////////
    //Объявлениу и открытие файла записи данных//
    ///////////////////////////////////////////*/
    QString fileDir = QString("data/data_")+QString(gh)+QString(".txt");
    QFile file1(fileDir);
    if(!file1.open(QIODevice::Append | QIODevice::Text) || !b.CrcIsCorrect())
    {
        item = new QStandardItem(QString("Ошибка открытия"));
        item->setBackground(QColor(255,128,0));
        model->setItem(numberRow, 9, item);

    }
    else
    {
        item = new QStandardItem(QString("Записан"));
        item->setBackground(QColor(0,128,128));
        model->setItem(numberRow, 9, item);
    }
    /*//////////////////////////////////////////////////////
    //Объявлениу и открытие файла записи данных калибровки//
    //////////////////////////////////////////////////////*/
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
    /*////////////////////////////////////////////
    //Анализ и запись в необходимые файлы данных//
    ////////////////////////////////////////////*/
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
    if(b.CrcIsCorrect() && MOStart)
    {
        Navarage++;
        avarageU1 = (avarageU1*(Navarage-1)+transf.f)/(Navarage);
        if(ui->checkDevice_1->checkState() && QString(gh) == ui->comboBoxAddressFunc3_1->currentText())
        {
            dNavarage[0] ++;
            dAvarageU1[0][0] = (dAvarageU1[0][0]*(dNavarage[0]-1)+transf.f)/(dNavarage[0]);
        }
        if(ui->checkDevice_2->checkState() && QString(gh) == ui->comboBoxAddressFunc3_2->currentText())
        {
            dNavarage[1] ++;
            dAvarageU1[1][0] = (dAvarageU1[1][0]*(dNavarage[1]-1)+transf.f)/(dNavarage[1]);
        }
        if(ui->checkDevice_3->checkState() && QString(gh) == ui->comboBoxAddressFunc3_3->currentText())
        {
            dNavarage[2] ++;
            dAvarageU1[2][0] = (dAvarageU1[2][0]*(dNavarage[2]-1)+transf.f)/(dNavarage[2]);
        }
        if(ui->checkDevice_4->checkState() && QString(gh) == ui->comboBoxAddressFunc3_4->currentText())
        {
            dNavarage[3] ++;
            dAvarageU1[3][0] = (dAvarageU1[3][0]*(dNavarage[3]-1)+transf.f)/(dNavarage[3]);
        }
        if(ui->checkDevice_5->checkState() && QString(gh) == ui->comboBoxAddressFunc3_5->currentText())
        {
            dNavarage[4] ++;
            dAvarageU1[4][0] = (dAvarageU1[4][0]*(dNavarage[4]-1)+transf.f)/(dNavarage[4]);
        }
    }
    transf.ie[0] = zData[13];//запись второй перемнной
    transf.ie[1] = zData[12];//запись второй перемнной
    transf.ie[2] = zData[11];//запись второй перемнной
    transf.ie[3] = zData[10];//запись второй перемнной
    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
    model->setItem(numberRow, 3, item);
    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
    if(b.CrcIsCorrect() && IsWriteCalibration) fileCalibration.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
    if(b.CrcIsCorrect() && MOStart)
    {
            avarageU2 = (avarageU2*(Navarage-1)+transf.f)/(Navarage);
            if(ui->checkDevice_1->checkState() && QString(gh) == ui->comboBoxAddressFunc3_1->currentText())
            {
                dAvarageU1[0][1] = (dAvarageU1[0][1]*(dNavarage[0]-1)+transf.f)/(dNavarage[0]);
            }
            if(ui->checkDevice_2->checkState() && QString(gh) == ui->comboBoxAddressFunc3_2->currentText())
            {
                dAvarageU1[1][1] = (dAvarageU1[1][1]*(dNavarage[1]-1)+transf.f)/(dNavarage[1]);
            }
            if(ui->checkDevice_3->checkState() && QString(gh) == ui->comboBoxAddressFunc3_3->currentText())
            {
                dAvarageU1[2][1] = (dAvarageU1[2][1]*(dNavarage[2]-1)+transf.f)/(dNavarage[2]);
            }
            if(ui->checkDevice_4->checkState() && QString(gh) == ui->comboBoxAddressFunc3_4->currentText())
            {
                dAvarageU1[3][1] = (dAvarageU1[3][1]*(dNavarage[3]-1)+transf.f)/(dNavarage[3]);
            }
            if(ui->checkDevice_5->checkState() && QString(gh) == ui->comboBoxAddressFunc3_5->currentText())
            {
                dAvarageU1[4][1] = (dAvarageU1[4][1]*(dNavarage[4]-1)+transf.f)/(dNavarage[4]);
            }
    }
    transf.ie[0] = zData[18];//запись третьей перемнной
    transf.ie[1] = zData[17];//запись третьей перемнной
    transf.ie[2] = zData[16];//запись третьей перемнной
    transf.ie[3] = zData[15];//запись третьей перемнной
    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
    model->setItem(numberRow, 4, item);
    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
    if(b.CrcIsCorrect() && MOStart)
    {
        avarageU3 = (avarageU3*(Navarage-1)+transf.f)/(Navarage);
        if(ui->checkDevice_1->checkState() && QString(gh) == ui->comboBoxAddressFunc3_1->currentText())
        {
            dAvarageU1[0][2] = (dAvarageU1[0][2]*(dNavarage[0]-1)+transf.f)/(dNavarage[0]);
        }
        if(ui->checkDevice_2->checkState() && QString(gh) == ui->comboBoxAddressFunc3_2->currentText())
        {
            dAvarageU1[1][2] = (dAvarageU1[1][2]*(dNavarage[1]-1)+transf.f)/(dNavarage[1]);
        }
        if(ui->checkDevice_3->checkState() && QString(gh) == ui->comboBoxAddressFunc3_3->currentText())
        {
            dAvarageU1[2][2] = (dAvarageU1[2][2]*(dNavarage[2]-1)+transf.f)/(dNavarage[2]);
        }
        if(ui->checkDevice_4->checkState() && QString(gh) == ui->comboBoxAddressFunc3_4->currentText())
        {
            dAvarageU1[3][2] = (dAvarageU1[3][2]*(dNavarage[3]-1)+transf.f)/(dNavarage[3]);
        }
        if(ui->checkDevice_5->checkState() && QString(gh) == ui->comboBoxAddressFunc3_5->currentText())
        {
            dAvarageU1[4][2] = (dAvarageU1[4][2]*(dNavarage[4]-1)+transf.f)/(dNavarage[4]);
        }
    }
    transf.ie[0] = zData[23];//запись четвертой перемнной
    transf.ie[1] = zData[22];//запись четвертой перемнной
    transf.ie[2] = zData[21];//запись четвертой перемнной
    transf.ie[3] = zData[20];//запись четвертой перемнной
    item = new QStandardItem(QString("%1").number(transf.f,'f',4));
    model->setItem(numberRow, 5, item);
    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
    if(b.CrcIsCorrect() && IsWriteCalibration) fileCalibration.write(QByteArray("%1").number(transf.f,'e',4)+QByteArray("\t"));
    if(b.CrcIsCorrect() && MOStart)
    {
        avarageU4 = (avarageU4*(Navarage-1)+transf.f)/(Navarage);
        if(ui->checkDevice_1->checkState() && QString(gh) == ui->comboBoxAddressFunc3_1->currentText())
        {
            dAvarageU1[0][3] = (dAvarageU1[0][3]*(dNavarage[0]-1)+transf.f)/(dNavarage[0]);
        }
        if(ui->checkDevice_2->checkState() && QString(gh) == ui->comboBoxAddressFunc3_2->currentText())
        {
            dAvarageU1[1][3] = (dAvarageU1[1][3]*(dNavarage[1]-1)+transf.f)/(dNavarage[1]);
        }
        if(ui->checkDevice_3->checkState() && QString(gh) == ui->comboBoxAddressFunc3_3->currentText())
        {
            dAvarageU1[2][3] = (dAvarageU1[2][3]*(dNavarage[2]-1)+transf.f)/(dNavarage[2]);
        }
        if(ui->checkDevice_4->checkState() && QString(gh) == ui->comboBoxAddressFunc3_4->currentText())
        {
            dAvarageU1[3][3] = (dAvarageU1[3][3]*(dNavarage[3]-1)+transf.f)/(dNavarage[3]);
        }
        if(ui->checkDevice_5->checkState() && QString(gh) == ui->comboBoxAddressFunc3_5->currentText())
        {
            dAvarageU1[4][3] = (dAvarageU1[4][3]*(dNavarage[4]-1)+transf.f)/(dNavarage[4]);
        }
    }

    item = new QStandardItem(QString("%1").number(PPos));
    model->setItem(numberRow, 6, item);
    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(PPos)+QByteArray("\t"));
    item = new QStandardItem(QString("%1").number(PNeg));
    model->setItem(numberRow, 7, item);
    if(b.CrcIsCorrect()) file1.write(QByteArray("%1").number(PNeg)+QByteArray("\n"));
    ui->labelU1->setText("U1="+QString("%1").number(avarageU1,'f',4)+" мВ");
    ui->labelU2->setText("U2="+QString("%1").number(avarageU2,'f',4)+" мВ");
    ui->labelU3->setText("U3="+QString("%1").number(avarageU3,'f',4)+" мВ");
    ui->labelU4->setText("U4="+QString("%1").number(avarageU4,'f',4)+" мВ");
    ui->tableView->setModel(model);
    ui->tableView->resizeRowsToContents();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->scrollToBottom();
    numberRow++;
    file1.close();
    fileCalibration.close();
}
void HartTester::readData()//read data
{


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
                if (int(ui->tabWidget->currentIndex())==1 && !timerFindDevice->isActive())
                {
                    displayData(b);
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
void HartTester::readData_m()
{
    int bytesAvaible = int(serial->bytesAvailable());
    char *buf = new char[bytesAvaible];
    serial->read(buf,bytesAvaible);
    serial->clear(QSerialPort::Input);
    int i = 0;
    QTextEdit *text_out;
    char sym;
    for(i=0; i<bytesAvaible; i++)
    {
        sym = buf[i];
        if((!IsPreamble) && nSymPreamble >2)
        {
            IsPreamble = false;
            ansGet_m[nSymHartAnswer] = sym;
            if(ansGet_m[nSymPreamble] == (char)0x82) LongFrame = 1;
            else LongFrame = 0;
            if(LongFrame == 0 && nSymHartAnswer == 3+nSymPreamble){
                nSymDataHartAnswer = ansGet_m[3+nSymPreamble];
                nSymHartAnswerTotal = nSymDataHartAnswer+5+nSymPreamble+2;
            }
            else if(LongFrame == 1 && nSymHartAnswer == 7+nSymPreamble){
                nSymDataHartAnswer = ansGet_m[7+nSymPreamble];
                nSymHartAnswerTotal = nSymDataHartAnswer+9+nSymPreamble+2;
            }
            nSymHartAnswer++;
            if (nSymHartAnswer == nSymHartAnswerTotal && nSymHartAnswer !=0 )
            {
                QByteArray outText = QByteArray(ansGet_m,nSymHartAnswer).toHex();
                int m = 0;
                for(m=2;m<outText.length();m=m+3){
                    outText = outText.insert(m," ");
                }
                if(int(ui->tabWidget->currentIndex())==0)
                {
                    ui->textEdit->setTextBackgroundColor(QColor(0,0,0));
                    ui->textEdit->setTextColor(QColor(255,255,255));
                    ui->textEdit->insertPlainText(outText);
                }


                answer b(nSymHartAnswer);
                b.createAnswer(ansGet_m,nSymHartAnswer);
                b.analysis();
                if(b.CrcIsCorrect())
                {
                    answerIsGet = true;
                    ui->lineEditStatus->setText("Верный ответ получен");
//                    ui->textEdit->setTextBackgroundColor(QColor(0,255,0));
//                    ui->textEdit->setTextColor(QColor(0,0,0));
//                    ui->textEdit->insertPlainText("CRC Ok");
//                    ui->textEdit->setTextBackgroundColor(QColor(255,255,255));
                }
                else
                {
                    ui->textEdit->setTextBackgroundColor(QColor(255,0,0));
//                    ui->textEdit->setTextColor(QColor(0,0,0));
//                    ui->textEdit->insertPlainText("CRC Bad");
//                    ui->textEdit->setTextBackgroundColor(QColor(255,255,255));
//                    ui->textEdit->setTextColor(QColor(0,0,0));
                }
                ui->textEdit->setTextColor(QColor(0,0,0));
                answerIsGet = true;
                if (int(ui->tabWidget->currentIndex())==1 && !timerFindDevice->isActive()) {
                    displayData(b);
                }

                if(!ui->checkAltView->checkState())
                {
                    ui->textEdit->setTextBackgroundColor(QColor(0,0,0));
                    ui->textEdit->setTextColor(QColor(255,255,255));
                    //ui->textEdit->insertPlainText(outText+QString(""));
                    if(int(ui->tabWidget->currentIndex())==0)
                    {
                        if(!b.CrcIsCorrect()){
                            ui->textEdit->setTextBackgroundColor(QColor(255,0,0));
                            ui->textEdit->setTextColor(QColor(0,0,0));
                            ui->textEdit->insertPlainText("CRC Bad");
                            ui->textEdit->setTextBackgroundColor(QColor(255,255,255));
                            ui->textEdit->setTextColor(QColor(0,0,0));
                        }else
                        {
                            ui->textEdit->setTextBackgroundColor(QColor(0,255,0));
                            ui->textEdit->setTextColor(QColor(0,0,0));
                            ui->textEdit->insertPlainText("CRC Ok");
                            ui->textEdit->setTextBackgroundColor(QColor(255,255,255));

                        }
                    ui->textEdit->setTextColor(QColor(0,0,0));
                    }
                }
                else
                {
                    answer b(nSymHartAnswer);
                    b.createAnswer(ansGet_m,nSymHartAnswer);
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
                            ui->textEdit->setTextBackgroundColor(QColor(0,0,0));
                            ui->textEdit->setTextColor(QColor(255,255,255));
                            ui->textEdit->insertPlainText(QString("Float ")+QString("\t%1>:").number(-3+int(n/1)+ReqData[0])+QString(" to ")+QString("\t%1>:").number(int(n/1)+ReqData[0])+QString(":\t")+QString("%1").number(transf.f)+QString("\n"));
                            ui->textEdit->setTextBackgroundColor(QColor(255,255,255));
                            ui->textEdit->setTextColor(QColor(0,0,0));
                        }
                    }

                }
            }
        }
        if((sym == (char)0x02||sym == (char)0x82)&& IsPreamble && nSymPreamble >2){//Ð’Ñ‹Ð´ÐµÐ»ÑÐµÐ¼ ÑÑ‚Ð°Ñ€Ñ‚Ð¾Ð²Ñ‹Ð¹ Ð±Ð°Ð¹Ñ‚
            IsPreamble = 0;
            //nSymHartAnswer++;
            for(int j = 0; j<nSymPreamble;j++)
            {
                ansGet_m[j] = (char)0xff;
            }
            ansGet_m[nSymPreamble] = sym;
            nSymHartAnswer = nSymPreamble+1;
        }
        if(sym == (char)0xff && IsPreamble)
        {
            nSymPreamble++;
        }
    }
}
void HartTester::clearText()//clear
{
    ui->textEdit->clear();
}
void HartTester::showTime(){
    if(!(serial->isOpen())){
        ui->textEdit->setText("Not Open");
    }
    HartTester::sendRequest();
}
void HartTester::startLoop()//timerstart
{
    timer->start(static_cast<int>(ui->doubleSpinBoxTimerSend->value()*1000));
    if(ui->comboBoxFunc->currentData().toInt() == 91){
        ui->buttonSend->setEnabled(false);
        ui->buttonTimerStart->setEnabled(false);
        ui->pushButton_7->setEnabled(true);
    }
}
void HartTester::stopLoop()//timer stop
{
    timer->stop();
}
void HartTester::on_comboBoxFunc_currentIndexChanged()//отклик на изменение функции
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
void HartTester::on_spinBox_valueChanged()//изменение длины преамбулы
{

    data91 = new unsigned char[int(ui->spinData91->value())+1];
    changedInByteExpected();
    //getRequestAddr();
    createRequestOut();
}
void HartTester::on_checkLongFrame_stateChanged()//отклик на длину фрейма
{
    data91 = new unsigned char[int(ui->spinData91->value())+1];
    changedInByteExpected();
    getRequestAddr(ui->checkLongFrame->checkState());
}
void HartTester::on_checkEnTextBrows_stateChanged(int arg1)//отклsючение текстовго редктора
{
    if(arg1==0){
        ui->textEdit->setEnabled(false);
        ui->buttonClear->setEnabled(false);
    }else{
        ui->textEdit->setEnabled(true);
        ui->buttonClear->setEnabled(true);
    }
}
void HartTester::on_lineEdit_2_textChanged(const QString &arg1)//ввод дданных
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
void HartTester::on_spinData91_valueChanged(int arg1)//разрешение ввода данных
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
void HartTester::on_lineFloat_1_textChanged()
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
void HartTester::on_lineFloat_2_textChanged()
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
void HartTester::on_lineFloat_3_textChanged()
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
void HartTester::on_lineFloat_4_textChanged()
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
void HartTester::on_lineFloat_5_textChanged()
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
void HartTester::on_lineFloat_6_textChanged()
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
void HartTester::create91Request()//создание 91 запроса
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
void HartTester::requestFunction3(QComboBox *f)
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
void HartTester::Function3Send()//send Function3
{
    connect(timerFunction3,&QTimer::timeout,this,&HartTester::sendTimerRequest);
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
void HartTester::sendTimerRequest()
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
       disconnect(timerFunction3,&QTimer::timeout,this,&HartTester::sendTimerRequest);
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
        HartTester::sendRequest();
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
        HartTester::sendRequest();
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
        HartTester::sendRequest();
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
        HartTester::sendRequest();
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
        HartTester::sendRequest();
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
void HartTester::sendTimerRequestLoop()
{
    if(allRequestIsSend)
    {
        Function3Send();
    }
}
void HartTester::startLoopFunction3()
{    
    ui->tabWidget->setTabEnabled(0,false);
    ui->tabWidget->setTabEnabled(2,false);
    connect(timerFunction3Loop,&QTimer::timeout,this,&HartTester::sendTimerRequestLoop);
    Function3Send();
    timerFunction3Loop->start(timerSendRequest);
}
void HartTester::stopLoopFunction3()
{
    ui->tabWidget->setTabEnabled(0,true);
    ui->tabWidget->setTabEnabled(2,true);
    disconnect(timerFunction3Loop,&QTimer::timeout,this,&HartTester::sendTimerRequestLoop);
    timerFunction3Loop->stop();
}
void HartTester::clearTable()
{
    model->clear();
    numberRow = 0;
}
void HartTester::on_tabWidget_currentChanged(int index)
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
void HartTester::on_lineAddrShort_textChanged()
{
    getRequestAddr(ui->checkLongFrame->checkState());
}
void HartTester::on_lineAddrLong_textChanged()
{
    getRequestAddr(ui->checkLongFrame->checkState());
}

void HartTester::on_checkDevice_1_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_1->currentText(), ui->checkDevice_1->checkState());
}
void HartTester::on_checkDevice_2_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_2->currentText(), ui->checkDevice_2->checkState());
}
void HartTester::on_checkDevice_3_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_3->currentText(), ui->checkDevice_3->checkState());
}
void HartTester::on_checkDevice_4_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_4->currentText(), ui->checkDevice_4->checkState());
}
void HartTester::on_checkDevice_5_stateChanged()
{
    showHideTableRow(ui->comboBoxAddressFunc3_5->currentText(), ui->checkDevice_5->checkState());
}
void HartTester::findDevice()//запуск таймера на поиск
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
void HartTester::sendFindRequest()//создание запроса на поиск
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
void HartTester::on_comboBoxAddress_highlighted()
{
        getRequestAddr(ui->checkLongFrame->checkState());
        ui->comboBoxAddressFunc3_1->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_2->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_3->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_4->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
}
void HartTester::on_comboBoxAddress_currentIndexChanged()
{
        getRequestAddr(ui->checkLongFrame->checkState());
        ui->comboBoxAddressFunc3_1->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_2->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_3->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_4->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
        ui->comboBoxAddressFunc3_5->setCurrentIndex(ui->comboBoxAddress->currentIndex());
}

void HartTester::spanRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationSpan);
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
void HartTester::indicateCalibrationSpan()
{

    if(answerIsGet)
    {
        ui->label_3->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationSpan);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_3->setStyleSheet(styleSheetCalibrationDefault);        
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationSpan);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::zeroRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZero);
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
void HartTester::indicateCalibrationZero()
{

    if(answerIsGet)
    {
        ui->label_4->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZero);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_4->setStyleSheet(styleSheetCalibrationDefault);        
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZero);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::zeroFirstVarRequest()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZeroFirstVar);
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
void HartTester::indicateCalibrationZeroFirstVar()
{

    if(answerIsGet)
    {
        ui->label_5->setStyleSheet(styleSheetCalibrationOk);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZeroFirstVar);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->label_5->setStyleSheet(styleSheetCalibrationBad);        
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateCalibrationZeroFirstVar);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::checkPassword()
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

void HartTester::calibrationFunctions(unsigned char *data,int numberData)
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
void HartTester::calibrationFunctionsGet(unsigned char *data1,int numberData1)
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

void HartTester::setAddress()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetAddress);
    unsigned char *dataChangeAddress = new unsigned char [2];
    QString textAddr = ui->comboBoxSetAddress->currentText();
    QByteArray text =QByteArray(textAddr.toLocal8Bit());
    QByteArray hex = QByteArray::fromHex(text);
    dataChangeAddress[0] = 0x03;
    dataChangeAddress[1] =(unsigned char)hex[0];
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void HartTester::setMode()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMode);
    unsigned char *dataChangeAddress = new unsigned char [2];
    int textMode = ui->comboBoxSetMode->currentData().toInt();
    dataChangeAddress[0] = 0x01;
    dataChangeAddress[1] =(unsigned char)textMode;
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void HartTester::setMaxValue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue);
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
void HartTester::setMaxValue_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue_2);
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
void HartTester::setMovingAverage_1()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_1);
    unsigned char *dataChangeAddress = new unsigned char [2];
    dataChangeAddress[0] = 0x0b;
    dataChangeAddress[1] = (unsigned char)ui->spinBoxMovingAverage_1->value();
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void HartTester::setMovingAverage_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_2);
    unsigned char *dataChangeAddress = new unsigned char [2];
    dataChangeAddress[0] = 0x0d;
    dataChangeAddress[1] = (unsigned char)ui->spinBoxMovingAverage_2->value();
    timerCalibration->start(50);
    calibrationFunctions(dataChangeAddress,2);
}
void HartTester::setA_40()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_40);
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
void HartTester::setA_41()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_41);
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
void HartTester::setA_42()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditBad);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_42);
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

void HartTester::getAddress()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetAddress);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x03;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void HartTester::getMode()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMode);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x01;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void HartTester::getMaxValue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x1e;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getMaxValue_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue_2);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x22;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getMovingAverage_1()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_1);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x0b;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void HartTester::getMovingAverage_2()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_2);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x0d;
    data[1] = 0x01;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,2);
}
void HartTester::getA_40()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_40);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x4e;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getA_41()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_41);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x52;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getA_42()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_42);
    unsigned char *data = new unsigned char [2];
    data[0] = 0x56;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getPressue()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetPressue);
    unsigned char *data = new unsigned char [2];
    data[0] = 0xcc;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}
void HartTester::getCurrent()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetCurrent);
    unsigned char *data = new unsigned char [2];
    data[0] = 0xc8;
    data[1] = 0x04;
    timerCalibration->start(50);
    calibrationFunctionsGet(data,5);
}

void HartTester::indicateSetAddress()
{
    if(answerIsGet)
    {
        ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetAddress);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->comboBoxSetAddress->setStyleSheet(styleSheetCalibrationComboBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetAddress);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetMode()
{
    if(answerIsGet)
    {
        ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMode);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->comboBoxSetMode->setStyleSheet(styleSheetCalibrationComboBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMode);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetMaxValue()
{
    if(answerIsGet)
    {
        ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetMaxValue_2()
{
    if(answerIsGet)
    {
        ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetMaxValue_2->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMaxValue_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetMovingAverage_1()
{
    if(answerIsGet)
    {
        ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_1);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_1);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetMovingAverage_2()
{
    if(answerIsGet)
    {
        ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->spinBoxMovingAverage_2->setStyleSheet(styleSheetCalibrationSpinBoxDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetMovingAverage_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetA_40()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_40);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_40->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_40);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetA_41()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_41);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_41->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_41);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateSetA_42()
{
    if(answerIsGet)
    {
        ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_42);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetA_42->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetA_42);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::indicateGetAddress()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        //answerTY b(inBytesExpected);
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        char f =zData[0];
        QByteArray outText = QByteArray(1,f).toHex();
        ui->lineEditGetAddress->setText(outText);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetAddress);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetAddress->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetAddress);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetMode()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMode);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMode->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMode);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetMaxValue()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;

        ui->lineEditMaxValueGet->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetMaxValue_2()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;

        ui->lineEditMaxValueGet_2->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMaxValue_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetMovingAverage_1()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        int f =zData[0];
        ui->lineEditGetMovingAverage_1->setText(QString().number(f));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_1);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMovingAverage_1->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_1);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetMovingAverage_2()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
        b.analysis();
        char *zData = new char [5];
        zData = b.getData();
        int f =zData[0];
        ui->lineEditGetMovingAverage_2->setText(QString().number(f));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_2);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetMovingAverage_2->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetMovingAverage_2);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetA_40()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_40);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_40->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_40);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetA_41()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_41);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_41->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_41);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetA_42()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_42);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetA_42->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetA_42);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetPressue()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetPressue);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetPressue->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetPressue);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}
void HartTester::indicateGetCurrent()
{
    if(answerIsGet)
    {
        countIndicateCalibration = 0;
        answer b(nSymHartAnswer);
        b.createAnswer(ansGet_m,nSymHartAnswer);
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
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetCurrent);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditGetCurrent->setText(QString("Bad Crc"));
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateGetCurrent);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::on_checkBoxFixedCurrent_stateChanged()
{
    //ui->spinBoxMovingAverage_1->setStyleSheet(styleSheetCalibrationSpinBoxBad);
    ui->buttonSetFixedCurrent->setEnabled(false);
    ui->lineEditSetFixedCurrent->setEnabled(false);
    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateStateFixedCurren);
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
void HartTester::indicateStateFixedCurren()
{
    if(answerIsGet)
    {
        //ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        ui->buttonSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        ui->lineEditSetFixedCurrent->setEnabled(ui->checkBoxFixedCurrent->checkState());
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateStateFixedCurren);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->buttonSetFixedCurrent->setEnabled(false);
        ui->lineEditSetFixedCurrent->setEnabled(false);
        //ui->lineEditSetMaxValue->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateStateFixedCurren);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::setValueFixedCurrent()
{

    connect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetValueFixedCurrent);
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
void HartTester::indicateSetValueFixedCurrent()
{
    if(answerIsGet)
    {
        ui->lineEditSetFixedCurrent->setStyleSheet(styleSheetCalibrationLineEditGood);
        countIndicateCalibration = 0;
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetValueFixedCurrent);
        timerCalibration->stop();
    }
    if(countIndicateCalibration>40)
    {
        countIndicateCalibration = 0;
        ui->lineEditSetFixedCurrent->setStyleSheet(styleSheetCalibrationLineEditDefault);
        disconnect(timerCalibration, &QTimer::timeout, this, &HartTester::indicateSetValueFixedCurrent);
        timerCalibration->stop();
    }
    countIndicateCalibration++;
}

void HartTester::on_checkBoxCakibrationMiddle_stateChanged()
{
    if(ui->checkBoxCakibrationMiddle->checkState())
    {
        ui->checkBoxCakibrationHigh->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationLow->setCheckState(Qt::Unchecked);
    }
}
void HartTester::on_checkBoxCakibrationLow_stateChanged()
{
    if(ui->checkBoxCakibrationLow->checkState())
    {
        ui->checkBoxCakibrationHigh->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationMiddle->setCheckState(Qt::Unchecked);
    }
}
void HartTester::on_checkBoxCakibrationHigh_stateChanged()
{
    if(ui->checkBoxCakibrationHigh->checkState())
    {
        ui->checkBoxCakibrationLow->setCheckState(Qt::Unchecked);
        ui->checkBoxCakibrationMiddle->setCheckState(Qt::Unchecked);
    }
}
void HartTester::findCoef()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
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
    U4n = UMiddle[2]*1000;
    U4l = ULow[2]*1000;
    U4h = UHigh[2]*1000;
    U1l = ULow[0]*1000;
    U1h = UHigh[0]*1000;
    U2l = ULow[1]*1000;
    U2h = UHigh[1]*1000;
    answerIsGet = true;
    answerAnalisys = true;
    requset1FindCoef = 0;
    requset2FindCoef = 0;
    requset3FindCoef = 0;
    requset4FindCoef = 0;
    requset5FindCoef = 0;
    countTimerFinc3 = 0;
    ui->buttonFindCoef->setEnabled(false);
    connect(timerFunction3,&QTimer::timeout,this,&HartTester::sendFindCoefRequest);
    timerFunction3->start(timerSendRequest);

    /*float U10 = ui->lineEditMaxValueGet_2->text().toFloat();
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
    }*/


}
void HartTester::sendFindCoefRequest()
{
    if(countTimerFinc3 >(3000/timerSendRequest))//timeout
    {
        countTimerFinc3 = 0;
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("НE УДАЛОСЬ ПОЛУЧИТЬ КОЭФФИЦИЕНТЫ");
        ui->buttonFindCoef->setEnabled(true);
        timerFunction3->stop();
        return void();
    }
    if(answerIsGet && !requset1FindCoef&&answerAnalisys)
    {   unsigned char *data = new unsigned char [2];
        data[0] = 0x1e;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset1FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset2FindCoef&&answerAnalisys)
    {    unsigned char *data = new unsigned char [2];
        data[0] = 0x26;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset2FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset3FindCoef&&answerAnalisys)
    {   unsigned char *data = new unsigned char [2];
        data[0] = 0x2a;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset3FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset4FindCoef&&answerAnalisys)
    {    unsigned char *data = new unsigned char [2];
        data[0] = 0x2e;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset4FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset5FindCoef&&answerAnalisys)
    {   unsigned char *data = new unsigned char [2];
        data[0] = 0x32;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset5FindCoef = true;
        return void();
    }
    countTimerFinc3++;
    if(answerIsGet)
    {
            countTimerFinc3 = 0;
            answerAnalisys = true;
            answer b(nSymHartAnswer);
            b.createAnswer(ansGet_m,nSymHartAnswer);
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
            //ui->lineEditGetA_40->setText(QString().number(transf.f));
            //ui->lineEditStatus->insert(QString().number(transf.f));
            if(requset1FindCoef&&!requset2FindCoef)
            {
                Pkstat = transf.f;
            }
            if(requset2FindCoef&&!requset3FindCoef)
            {
                Uk1h = transf.f;
            }
            if(requset3FindCoef&&!requset4FindCoef)
            {
                Uk2h = transf.f;
            }
            if(requset4FindCoef&&!requset5FindCoef)
            {
                Uk10 = transf.f;
            }
            if(requset4FindCoef&&requset5FindCoef)
            {
                Uk20 = transf.f;
            }
            if(requset5FindCoef&&requset4FindCoef&&requset3FindCoef&&requset2FindCoef&&requset1FindCoef)
            {
                float U10 = Uk10;
                float U20 = Uk20;
//                U10 = 3392;
//                U20 = 2918;
//                Uk1h = 19490;
//                Uk2h = 11098;
                float K1 = Pkstat/(Uk1h-U10);
                float K2 = Pkstat/(Uk2h-U20);
                float a42 = 0,a41 = 0,a40 = 0;
                a42 = -(K1*U10*U4h - K2*U20*U4h - K1*U10*U4l + K2*U20*U4l + K1*U1h*U4l - K1*U4h*U1l - K2*U2h*U4l + K2*U4h*U2l - K1*U1h*U4n + K2*U2h*U4n + K1*U1l*U4n - K2*U2l*U4n)/((U4h - U4l)*(U4h - U4n)*(U4l - U4n));
                a41 = (- a42*U4l*U4l + a42*U4n*U4n + K1*(U10 - U1l) - K2*(U20 - U2l))/(U4l - U4n);
                a40 = - a42*U4n*U4n - a41*U4n;
                ui->lineEditSetA_40->setText(QString().number(a42));
                ui->lineEditSetA_41->setText(QString().number(a41));
                ui->lineEditSetA_42->setText(QString().number(a40));
                disconnect(timerFunction3, &QTimer::timeout, this, &HartTester::sendFindCoefRequest);
                ui->buttonFindCoef->setEnabled(true);
                timerFunction3->stop();
                if(ui->checkBoxParameterOutput->checkState())
                {
                    setDialog.show();
                    setDialog.dialogData(Pkstat,Uk1h,Uk2h,U10,U20,U4l,U4n,U4h,U1l,U1h,U2l,U2h);
                }
                countTimerFinc3 = 0;
            }
    }
}
void HartTester::clearCalibrationData()
{
    removeFile(ui->comboBoxAddress->currentText());
}


void HartTester::on_spinBox_2_valueChanged(int arg1)
{
        timerSendRequest = arg1;
}
void HartTester::setPressue()
{
    PPos = ui->spinBoxPressuePos->value();
    PNeg = ui->spinBoxPressueNeg->value();
}

void HartTester::on_spinBoxPressueNeg_valueChanged(int arg1)
{
    if(ui->checkBoxSamePressue->checkState() && ui->spinBoxPressuePos->value() != ui->spinBoxPressueNeg->value())
    {
        ui->spinBoxPressuePos->setValue(ui->spinBoxPressueNeg->value());
    }
}
void HartTester::on_spinBoxPressuePos_valueChanged(int arg1)
{
    if(ui->checkBoxSamePressue->checkState() && ui->spinBoxPressuePos->value()!=ui->spinBoxPressueNeg->value())
    {
        ui->spinBoxPressueNeg->setValue(ui->spinBoxPressuePos->value());
    }
}
void HartTester::MO()
{
    MOStart = true;
}
void HartTester::clearMO()
{
    MOStart = false;
    avarageU1 = 0;
    avarageU2 = 0;
    avarageU3 = 0;
    avarageU4 = 0;
    Navarage = 0;
    for(int i = 0; i<5;i++)
    {
        for(int j = 0; j<4;j++)
        {
            dAvarageU1[i][j] = 0;
        }
        dNavarage[i] = 0;
    }
    ui->labelU1->setText("U1="+QString("%1").number(avarageU1,'f',4)+" мВ");
    ui->labelU2->setText("U2="+QString("%1").number(avarageU2,'f',4)+" мВ");
    ui->labelU3->setText("U3="+QString("%1").number(avarageU3,'f',4)+" мВ");
    ui->labelU4->setText("U4="+QString("%1").number(avarageU4,'f',4)+" мВ");
}
void HartTester::addDataCoef()
{
    float *dataCoef = new float [6];
    dataCoef[0]=avarageU1*1000;
    dataCoef[1]=avarageU2*1000;
    dataCoef[2]=avarageU3*1000;
    dataCoef[3]=avarageU4*1000;
    dataCoef[4]=ui->spinBoxPressuePos->value()*1000;
    dataCoef[5]=ui->spinBoxPressueNeg->value()*1000;
    deviceCoef.addLine(dataCoef);
    /*///////////////////////////////////////////
    //Объявлениу и открытие файла записи данных МНК//
    ///////////////////////////////////////////*/

    QString fileDirMNK = QString("data/dataMNK_")+QString(ui->comboBoxAddress->currentText())+QString(".txt");
    QFile fileMNK(fileDirMNK);
    if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
    {
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
        ui->lineEditStatus->setText("Файл калибровки МНК не записан");

    }
    else
    {
        fileMNK.write(QByteArray("%1").number(avarageU1,'e',4)+QByteArray("\t"));
        fileMNK.write(QByteArray("%1").number(avarageU2,'e',4)+QByteArray("\t"));
        fileMNK.write(QByteArray("%1").number(avarageU3,'e',4)+QByteArray("\t"));
        fileMNK.write(QByteArray("%1").number(avarageU4,'e',4)+QByteArray("\t"));
        fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
        fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
        ui->lineEditStatus->setText("Файл калибровки МНК записан");
    }
    fileMNK.close();



    if(ui->checkDevice_1->checkState())
    {
        QString fileDirMNK = QString("data/dataMNKNew_")+QString(ui->comboBoxAddressFunc3_1->currentText())+QString(".txt");
        QFile fileMNK(fileDirMNK);
        if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
        {
            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
            ui->lineEditStatus->setText("Файл калибровки МНК не записан");

        }
        else
        {
            fileMNK.write(QByteArray("%1").number(dAvarageU1[0][0],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[0][1],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[0][2],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[0][3],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
            ui->lineEditStatus->setText("Файл калибровки МНК записан");
        }
        fileMNK.close();
    }
    if(ui->checkDevice_2->checkState())
    {
        QString fileDirMNK = QString("data/dataMNKNew_")+QString(ui->comboBoxAddressFunc3_2->currentText())+QString(".txt");
        QFile fileMNK(fileDirMNK);
        if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
        {
            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
            ui->lineEditStatus->setText("Файл калибровки МНК не записан");

        }
        else
        {
            fileMNK.write(QByteArray("%1").number(dAvarageU1[1][0],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[1][1],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[1][2],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[1][3],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
            ui->lineEditStatus->setText("Файл калибровки МНК записан");
        }
        fileMNK.close();
    }
    if(ui->checkDevice_3->checkState())
    {
        QString fileDirMNK = QString("data/dataMNKNew_")+QString(ui->comboBoxAddressFunc3_3->currentText())+QString(".txt");
        QFile fileMNK(fileDirMNK);
        if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
        {
            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
            ui->lineEditStatus->setText("Файл калибровки МНК не записан");

        }
        else
        {
            fileMNK.write(QByteArray("%1").number(dAvarageU1[2][0],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[2][1],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[2][2],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[2][3],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
            ui->lineEditStatus->setText("Файл калибровки МНК записан");
        }
        fileMNK.close();
    }
    if(ui->checkDevice_4->checkState())
    {
        QString fileDirMNK = QString("data/dataMNKNew_")+QString(ui->comboBoxAddressFunc3_4->currentText())+QString(".txt");
        QFile fileMNK(fileDirMNK);
        if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
        {
            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
            ui->lineEditStatus->setText("Файл калибровки МНК не записан");

        }
        else
        {
            fileMNK.write(QByteArray("%1").number(dAvarageU1[3][0],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[3][1],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[3][2],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[3][3],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
            ui->lineEditStatus->setText("Файл калибровки МНК записан");
        }
        fileMNK.close();
    }
    if(ui->checkDevice_5->checkState())
    {
        QString fileDirMNK = QString("data/dataMNKNew_")+QString(ui->comboBoxAddressFunc3_5->currentText())+QString(".txt");
        QFile fileMNK(fileDirMNK);
        if(!fileMNK.open(QIODevice::Append | QIODevice::Text))
        {
            ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
            ui->lineEditStatus->setText("Файл калибровки МНК не записан");

        }
        else
        {
            fileMNK.write(QByteArray("%1").number(dAvarageU1[4][0],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[4][1],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[4][2],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(dAvarageU1[4][3],'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressuePos->value(),'e',4)+QByteArray("\t"));
            fileMNK.write(QByteArray("%1").number(ui->spinBoxPressueNeg->value(),'e',4)+QByteArray("\n"));
            ui->lineEditStatus->setText("Файл калибровки МНК записан");
        }
        fileMNK.close();
    }

}
void HartTester::findCoefMNK()
{
    if(!serial->isOpen()){
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("ПОРТ НЕ ОТКРЫТ");
        return void();
    }
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
    U4n = UMiddle[2]*1000;
    U4l = ULow[2]*1000;
    U4h = UHigh[2]*1000;
    U1l = ULow[0]*1000;
    U1h = UHigh[0]*1000;
    U2l = ULow[1]*1000;
    U2h = UHigh[1]*1000;
    answerIsGet = true;
    answerAnalisys = true;
    requset1FindCoef = 0;
    countTimerFinc3 = 0;
    ui->buttonFindCoefMNK->setEnabled(false);
    connect(timerFunction3,&QTimer::timeout,this,&HartTester::sendFindCoefRequestMNK);
    timerFunction3->start(timerSendRequest);
}
void HartTester::updateDataMNK()
{
    deviceCoef.clearCalibration();
    QString fileDirMNK = QString("data/dataMNK_")+ui->comboBoxAddress->currentText()+QString(".txt");
    QFile fileMNK(fileDirMNK);
    if(!fileMNK.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->lineEditStatus->setText("НЕ ОТКРЫТ МНК");
    }
    else
    {

    }
    QString dataMNK(fileMNK.readAll());
    fileMNK.close();
    QStringRef subString;
    float* calcDataFile = new float[6];
    int k = 0;
    int len = 0;
    int countVar = 0;
    int pos = 0;
    float f;
    for(k=0;k<dataMNK.length();k++)
    {
        if(dataMNK[k] == 0x0009 ||(countVar%6==5 && dataMNK[k] == 0x000a))
        {
            subString = QStringRef(&dataMNK, pos, len);
            if (countVar%6==0) {
                f = subString.toFloat();
                calcDataFile[0] = f*1000;
            }
            if (countVar%6==1) {
                f = subString.toFloat();
                calcDataFile[1] = f*1000;
            }
            if (countVar%6==2) {
                f = subString.toFloat();
                calcDataFile[2] = f*1000;
            }
            if (countVar%6==3) {
                f = subString.toFloat();
                calcDataFile[3] = f*1000;
            }
            if (countVar%6==4) {
                f = subString.toFloat();
                calcDataFile[4] = f*1000;
            }
            if (countVar%6==5) {
                f = subString.toFloat();
                calcDataFile[5] = f*1000;
                deviceCoef.addLine(calcDataFile);
            }
            pos =k+1;
            len =0;
            countVar++;
            continue;
        }
        len++;
    }

}

void HartTester::sendFindCoefRequestMNK()
{
    if(countTimerFinc3 >(3000/timerSendRequest))//timeout
    {
        countTimerFinc3 = 0;
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("НE УДАЛОСЬ ПОЛУЧИТЬ КОЭФФИЦИЕНТЫ");
        ui->buttonFindCoef->setEnabled(true);
        timerFunction3->stop();
        return void();
    }
    if(answerIsGet && !requset1FindCoef&&answerAnalisys)
    {   unsigned char *data = new unsigned char [2];
        data[0] = 0x1e;
        data[1] = 0x04;
        calibrationFunctionsGet(data,5);
        answerAnalisys = false;
        requset1FindCoef = true;
        return void();
    }
    countTimerFinc3++;
    if(answerIsGet)
    {
            countTimerFinc3 = 0;
            answerAnalisys = true;
            answer b(nSymHartAnswer);
            b.createAnswer(ansGet_m,nSymHartAnswer);
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
            //ui->lineEditGetA_40->setText(QString().number(transf.f));
            //ui->lineEditStatus->insert(QString().number(transf.f));
            if(requset1FindCoef)
            {
                Pkstat = transf.f;
            }
            if(requset1FindCoef)
            {

                /*deviceCoef.clearCalibration();
                float *emulatedData = new float[6];
                emulatedData[0] = 1962;
                emulatedData[1] = 2705;
                emulatedData[2] = 1019800;
                emulatedData[3] = 890956;
                emulatedData[4] = 0;
                emulatedData[5] = 0;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 5146;
                emulatedData[1] = 5870;
                emulatedData[2] = 1019710;
                emulatedData[3] = 890624;
                emulatedData[4] = 20000;
                emulatedData[5] = 20000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 8323;
                emulatedData[1] = 9031;
                emulatedData[2] = 1019450;
                emulatedData[3] = 890424;
                emulatedData[4] = 40000;
                emulatedData[5] = 40000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 11490;
                emulatedData[1] = 12189;
                emulatedData[2] = 1019169;
                emulatedData[3] = 890310;
                emulatedData[4] = 60000;
                emulatedData[5] = 60000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 14646;
                emulatedData[1] = 15343;
                emulatedData[2] = 1018839;
                emulatedData[3] = 890226;
                emulatedData[4] = 80000;
                emulatedData[5] = 80000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 17793;
                emulatedData[1] = 18491;
                emulatedData[2] = 1018800;
                emulatedData[3] = 890216;
                emulatedData[4] = 100000;
                emulatedData[5] = 100000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 17793;
                emulatedData[1] = 18491;
                emulatedData[2] = 1018800;
                emulatedData[3] = 890216;
                emulatedData[4] = 100000;
                emulatedData[5] = 100000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 14651;
                emulatedData[1] = 15347;
                emulatedData[2] = 1018600;
                emulatedData[3] = 890000;
                emulatedData[4] = 80000;
                emulatedData[5] = 80000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 11498;
                emulatedData[1] = 12197;
                emulatedData[2] = 1018600;
                emulatedData[3] = 889882;
                emulatedData[4] = 60000;
                emulatedData[5] = 60000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 8333;
                emulatedData[1] = 9041;
                emulatedData[2] = 1018600;
                emulatedData[3] = 889796;
                emulatedData[4] = 40000;
                emulatedData[5] = 40000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 5158;
                emulatedData[1] = 5879;
                emulatedData[2] = 1018600;
                emulatedData[3] = 889796;
                emulatedData[4] = 20000;
                emulatedData[5] = 20000;
                deviceCoef.addLine(emulatedData);
                emulatedData[0] = 1973;
                emulatedData[1] = 2715;
                emulatedData[2] = 1018600;
                emulatedData[3] = 889796;
                emulatedData[4] = 0;
                emulatedData[5] = 0;
                deviceCoef.addLine(emulatedData);*/
//                if(ui->checkBoxFromFile->checkState())
//                {
//                    updateDataMNK();//загрузка данных из файла в класс
//                }
                updateDataMNK();//загрузка данных из файла в класс
                float U10 = deviceCoef.getU10();
                float U20 = deviceCoef.getU20();
                Uk1h = deviceCoef.getU1h(Pkstat);
                Uk2h = deviceCoef.getU2h(Pkstat);
                //U10 = 2190;
                //U20 = 2904;
                //Uk1h = 18041;
                //Uk2h = 18497;

                float K1 = Pkstat/(Uk1h-U10);
                float K2 = Pkstat/(Uk2h-U20);
                float a42 = 0,a41 = 0,a40 = 0;
                a42 = -(K1*U10*U4h - K2*U20*U4h - K1*U10*U4l + K2*U20*U4l + K1*U1h*U4l - K1*U4h*U1l - K2*U2h*U4l + K2*U4h*U2l - K1*U1h*U4n + K2*U2h*U4n + K1*U1l*U4n - K2*U2l*U4n)/((U4h - U4l)*(U4h - U4n)*(U4l - U4n));
                a41 = (- a42*U4l*U4l + a42*U4n*U4n + K1*(U10 - U1l) - K2*(U20 - U2l))/(U4l - U4n);
                a40 = - a42*U4n*U4n - a41*U4n;
                ui->lineEditSetA_40->setText(QString().number(a42));
                ui->lineEditSetA_41->setText(QString().number(a41));
                ui->lineEditSetA_42->setText(QString().number(a40));
                disconnect(timerFunction3, &QTimer::timeout, this, &HartTester::sendFindCoefRequestMNK);
                ui->buttonFindCoefMNK->setEnabled(true);
                timerFunction3->stop();
                if(ui->checkBoxParameterOutput->checkState())
                {
                    setDialog.show();
                    setDialog.dialogData(Pkstat,Uk1h,Uk2h,U10,U20,U4l,U4n,U4h,U1l,U1h,U2l,U2h);
                }
                countTimerFinc3 = 0;
                /*///////////////////////////////////////////
                //Объявлениу и открытие файла записи данных МНК//
                ///////////////////////////////////////////*/

                QString fileDirCoef = QString("data/koefCalibration_")+QString(ui->comboBoxAddress->currentText())+QString(".txt");
                QFile fileCoef(fileDirCoef);
                if(!fileCoef.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgba(255,64,0);color:white}");
                    ui->lineEditStatus->setText("Файл коэффициентов не записан");

                }
                else
                {
                    fileCoef.write(QByteArray("Pmax = ")+QByteArray("%1").number(Pkstat/1000)+QByteArray(", кПа\n"));
                    fileCoef.write(QByteArray("Uk1h = ")+QByteArray("%1").number(Uk1h/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("Uk2h = ")+QByteArray("%1").number(Uk2h/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U10 = ")+QByteArray("%1").number(U10/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U20 = ")+QByteArray("%1").number(U20/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U4l = ")+QByteArray("%1").number(U4l/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U4n = ")+QByteArray("%1").number(U4n/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U4h = ")+QByteArray("%1").number(U4h/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U1l = ")+QByteArray("%1").number(U1l/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U1h = ")+QByteArray("%1").number(U1h/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U2l = ")+QByteArray("%1").number(U2l/1000)+QByteArray(", мВ\n"));
                    fileCoef.write(QByteArray("U2h = ")+QByteArray("%1").number(U2h/1000)+QByteArray(", мВ\n"));

                    fileCoef.write(QByteArray("K1 = ")+QByteArray("%1").number(K1)+QByteArray("\n"));
                    fileCoef.write(QByteArray("K2 = ")+QByteArray("%1").number(K2)+QByteArray("\n"));
                    fileCoef.write(QByteArray("a42 = ")+QByteArray("%1").number(a40)+QByteArray("\n"));
                    fileCoef.write(QByteArray("a41 = ")+QByteArray("%1").number(a41)+QByteArray("\n"));
                    fileCoef.write(QByteArray("a40 = ")+QByteArray("%1").number(a42)+QByteArray("\n"));
                    ui->lineEditStatus->setText("Файл калибровки МНК записан");
                }
                fileCoef.close();
                deviceCoef.clearCalibration();
            }
    }
}
void HartTester::testFunc(float* g)
{
    ui->lineEditStatus->setText(QString().number(g[0]));
    ui->lineEditStatus->insert(QString().number(g[1]));
    ui->lineEditStatus->insert(QString().number(g[2]));
    ui->lineEditStatus->insert(QString().number(g[3]));
    Uk10 = g[0];
    Uk20 = g[1];
    Uk1h = g[2];
    Uk2h = g[3];
    answerIsGet = true;
    answerAnalisys = true;
    requset1FindCoef = 0;
    requset2FindCoef = 0;
    requset3FindCoef = 0;
    requset4FindCoef = 0;
    countTimerFinc3 = 0;
    connect(timerFunction3,&QTimer::timeout,this,&HartTester::downloadCoef);
    timerFunction3->start(timerSendRequest);
    setDialog.close();
}
void HartTester::downloadCoef()
{
    if(countTimerFinc3 >(3000/timerSendRequest))//timeout
    {
        countTimerFinc3 = 0;
        ui->lineEditStatus->setStyleSheet("QLineEdit{background-color :rgb(255,0,0);color:white;font:bold}");
        ui->lineEditStatus->setText("НE УДАЛОСЬ ЗАГРУЗИТЬ КОЭФФИЦИЕНТЫ");
        timerFunction3->stop();
        return void();
    }
    if(answerIsGet && !requset1FindCoef&&answerAnalisys)
    {
        union{
            float f;
            char ie[4];

        }transf;
        transf.f = Uk1h;
        unsigned char *maxValue = new unsigned char [5];
        maxValue[0] = 0x26;
        maxValue[1] = transf.ie[3];
        maxValue[2] = transf.ie[2];
        maxValue[3] = transf.ie[1];
        maxValue[4] = transf.ie[0];
        calibrationFunctions(maxValue,5);
        answerAnalisys = false;
        requset1FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset2FindCoef&&answerAnalisys)
    {
        union{
            float f;
            char ie[4];

        }transf;
        transf.f = Uk2h;
        unsigned char *maxValue = new unsigned char [5];
        maxValue[0] = 0x2a;
        maxValue[1] = transf.ie[3];
        maxValue[2] = transf.ie[2];
        maxValue[3] = transf.ie[1];
        maxValue[4] = transf.ie[0];
        calibrationFunctions(maxValue,5);
        answerAnalisys = false;
        requset2FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset3FindCoef&&answerAnalisys)
    {
        union{
            float f;
            char ie[4];

        }transf;
        transf.f = Uk10;
        unsigned char *maxValue = new unsigned char [5];
        maxValue[0] = 0x2e;
        maxValue[1] = transf.ie[3];
        maxValue[2] = transf.ie[2];
        maxValue[3] = transf.ie[1];
        maxValue[4] = transf.ie[0];
        calibrationFunctions(maxValue,5);
        answerAnalisys = false;
        requset3FindCoef = true;
        return void();
    }
    if(answerIsGet && !requset4FindCoef&&answerAnalisys)
    {
        union{
            float f;
            char ie[4];

        }transf;
        transf.f = Uk20;
        unsigned char *maxValue = new unsigned char [5];
        maxValue[0] = 0x32;
        maxValue[1] = transf.ie[3];
        maxValue[2] = transf.ie[2];
        maxValue[3] = transf.ie[1];
        maxValue[4] = transf.ie[0];
        calibrationFunctions(maxValue,5);
        answerAnalisys = false;
        requset4FindCoef = true;
        return void();
    }
    countTimerFinc3++;
    if(answerIsGet)
    {
            countTimerFinc3 = 0;
            answerAnalisys = true;
            answer b(nSymHartAnswer);
            b.createAnswer(ansGet_m,nSymHartAnswer);
            b.analysis();
            if(requset4FindCoef&&requset3FindCoef&&requset2FindCoef&&requset1FindCoef)
            {
                disconnect(timerFunction3, &QTimer::timeout, this, &HartTester::downloadCoef);
                ui->lineEditStatus->setText("КОЭФФИЦИЕНТЫ ЗАГРУЖЕНЫ");
                timerFunction3->stop();
                countTimerFinc3 = 0;
            }
    }
}
