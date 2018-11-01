#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialog.h"
#include <QSerialPort>
#include <QLineEdit>
#include <QComboBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectCOM();
    void closeCOM();
    void sendRequest();

    void showTime();
    void sendTimerRequest();
    void sendTimerRequestLoop();
    void sendFindRequest();

    void readData();
    void clearText();
    void startLoop();
    void stopLoop();
    void on_comboBoxFunc_currentIndexChanged();
    void on_spinBox_valueChanged();
    void on_checkLongFrame_stateChanged();
    void on_checkEnTextBrows_stateChanged(int arg1);
    void on_lineEdit_2_textChanged(const QString &arg1);
    void on_spinData91_valueChanged(int arg1);
    void on_lineFloat_1_textChanged();
    void on_lineFloat_2_textChanged();
    void on_lineFloat_3_textChanged();
    void on_lineFloat_4_textChanged();
    void on_lineFloat_5_textChanged();
    void on_lineFloat_6_textChanged();
    void create91Request();
    void Function3Send();
    void startLoopFunction3();
    void stopLoopFunction3();
    void clearTable();
    void on_tabWidget_currentChanged(int index);
    void createRequestOut(bool tr);
    void getRequestAddr(bool b);
    void changedInByteExpected();    
    void showHideTableRow(QString r, bool t);
    void ResetAddressInit();
    void spanRequest();
    void zeroRequest();
    void zeroFirstVarRequest();
    void on_lineAddrShort_textChanged();
    void on_lineAddrLong_textChanged();

    void on_checkDevice_1_stateChanged();
    void on_checkDevice_2_stateChanged();
    void on_checkDevice_3_stateChanged();
    void on_checkDevice_4_stateChanged();
    void on_checkDevice_5_stateChanged();

    void findDevice();

    void on_comboBoxAddress_highlighted();

    void on_comboBoxAddress_currentIndexChanged();

    void indicateCalibrationSpan();
    void indicateCalibrationZero();
    void indicateCalibrationZeroFirstVar();
    void checkPassword();
    void calibrationFunctions(unsigned char *data,int numberData);
    void calibrationFunctionsGet(unsigned char *data1,int numberData1);

    void setAddress();
    void setMode();
    void setMaxValue();
    void setMaxValue_2();
    void setMovingAverage_1();
    void setMovingAverage_2();
    void setA_40();
    void setA_41();
    void setA_42();

    void getAddress();
    void getMode();
    void getMaxValue();
    void getMaxValue_2();
    void getMovingAverage_1();
    void getMovingAverage_2();
    void getA_40();
    void getA_41();
    void getA_42();
    void getCurrent();
    void getPressue();

    void indicateSetAddress();
    void indicateSetMode();
    void indicateSetMaxValue();
    void indicateSetMaxValue_2();
    void indicateSetMovingAverage_1();    
    void indicateSetMovingAverage_2();    
    void indicateSetA_40();
    void indicateSetA_41();    
    void indicateSetA_42();

    void indicateGetAddress();
    void indicateGetMode();
    void indicateGetMaxValue();
    void indicateGetMaxValue_2();
    void indicateGetMovingAverage_1();
    void indicateGetMovingAverage_2();
    void indicateGetA_40();
    void indicateGetA_41();
    void indicateGetA_42();
    void indicateGetCurrent();
    void indicateGetPressue();


    void on_checkBoxFixedCurrent_stateChanged();
    void indicateStateFixedCurren();

    void setValueFixedCurrent();
    void indicateSetValueFixedCurrent();

    void on_checkBoxCakibrationMiddle_stateChanged();
    void on_checkBoxCakibrationLow_stateChanged();
    void on_checkBoxCakibrationHigh_stateChanged();

    void updateCom();
    void findCoef();
    void clearCalibrationData();
    void on_spinBox_2_valueChanged(int arg1);
    void requestFunction3(QComboBox *f);
    void saveSettings();
    void downloadSettings();
    void connections();
    void aboutHartTester();
    void aboutQt();
private:
    Dialog setDialog;
    Ui::MainWindow *ui;
    QSerialPort *serial = nullptr;
    QTimer *timer = nullptr;
    QTimer *timerFunction3 = nullptr;
    QTimer *timerFunction3Loop = nullptr;
    QTimer *timerFindDevice = nullptr;
    QTimer *timerCalibration = nullptr;
};

#endif // MAINWINDOW_H
