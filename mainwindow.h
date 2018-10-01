#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void on_comboBoxFunc_currentIndexChanged(int index);
    void on_spinBox_valueChanged(int arg1);
    void on_checkLongFrame_stateChanged(int arg1);
    void on_checkEnTextBrows_stateChanged(int arg1);
    void on_lineEdit_2_textChanged(const QString &arg1);
    void on_spinData91_valueChanged(int arg1);
    void on_lineFloat_1_textChanged(const QString &arg1);
    void on_lineFloat_2_textChanged(const QString &arg1);
    void on_lineFloat_3_textChanged(const QString &arg1);
    void on_lineFloat_4_textChanged(const QString &arg1);
    void on_lineFloat_5_textChanged(const QString &arg1);
    void on_lineFloat_6_textChanged(const QString &arg1);
    void create91Request();
    void Function3Send();
    void startLoopFunction3();
    void stopLoopFunction3();
    void clearTable();
    void on_tabWidget_currentChanged(int index);
    void createRequestOut(bool tr);
    void getRequestAddr();
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

    void on_comboBoxAddress_highlighted(const QString &arg1);

    void on_comboBoxAddress_currentIndexChanged(int index);

    void indicateCalibrationSpan();
    void indicateCalibrationZero();
    void indicateCalibrationZeroFirstVar();
private:
    Ui::MainWindow *ui;
    QSerialPort *serial = nullptr;
    QTimer *timer = nullptr;
    QTimer *timerFunction3 = nullptr;
    QTimer *timerFunction3Loop = nullptr;
    QTimer *timerFindDevice = nullptr;
    QTimer *timerCalibration = nullptr;
};

#endif // MAINWINDOW_H
