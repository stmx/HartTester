#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
QT_BEGIN_NAMESPACE
namespace Ui {
    class Dialog;
}
QT_END_NAMESPACE
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    void dialogData(float Pstat,float Uk1h,float Uk2h, float Uk10, float Uk20,float U4l,float U4m,float U4h, float U1l, float U1h, float U2l, float U2h);
    ~Dialog();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
