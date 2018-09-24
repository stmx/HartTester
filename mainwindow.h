#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLineEdit>

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
    void on_pushButton_clicked();
    void on_buttonClose_clicked();
    void on_buttonSend_clicked();

    void showTime();
    void sendTimerRequest();
    void sendTimerRequestLoop();
    void sendFindRequest();

    void readData();
    void on_buttonClear_clicked();
    void on_buttonTimerStart_clicked();
    void on_buttonTimerStop_clicked();
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
    void on_pushButton_7_clicked();
    void on_buttonFunction3Send_clicked();
    void on_buttonFunction3Loop_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
    void on_tabWidget_currentChanged(int index);
    void createRequestOut(bool tr);
    void getRequestAddr();
    void changedInByteExpected();    
    void showHideTableRow(QLineEdit *r, bool t);

    void on_lineAddrShort_textChanged();
    void on_lineAddrLong_textChanged();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_checkDevice_1_stateChanged();
    void on_checkDevice_2_stateChanged();
    void on_checkDevice_3_stateChanged();
    void on_checkDevice_4_stateChanged();
    void on_checkDevice_5_stateChanged();

    void on_buttonFindDevice_clicked();

    void on_comboBoxAddress_highlighted(const QString &arg1);

    void on_comboBoxAddress_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QSerialPort *serial = nullptr;
    QTimer *timer = nullptr;
    QTimer *timerFunction3 = nullptr;
    QTimer *timerFunction3Loop = nullptr;
    QTimer *timerFindDevice = nullptr;
};

#endif // MAINWINDOW_H
