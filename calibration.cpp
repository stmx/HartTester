#include "calibration.h"

calibration::calibration()
{
    U1sumx = 0;
    U1sumy = 0;
    U1sumxy = 0;
    U1sumx2 = 0;
    U2sumx = 0;
    U2sumy = 0;
    U2sumxy = 0;
    U2sumx2 = 0;
    K1 = 0;
    b1 = 0;
    K2 = 0;
    b2 = 0;
    n = 0;
}
void calibration::addLine(float *b)
{
    U1sumx += b[4];
    U1sumy += b[0];
    U1sumxy += b[0]*b[4];
    U1sumx2 += b[4]*b[4];
    U2sumx += b[5];
    U2sumy += b[1];
    U2sumxy += b[1]*b[5];
    U2sumx2 += b[5]*b[5];
    n++;
    if(n>1)
    {
        K1 = (n*U1sumxy-U1sumx*U1sumy)/(n*U1sumx2-U1sumx*U1sumx);
        b1 = (U1sumy-K1*U1sumx)/(n);
        U10 = b1;
    }
    if(n>1)
    {
        K2 = (n*U2sumxy-U2sumx*U2sumy)/(n*U2sumx2-U2sumx*U2sumx);
        b2 = (U2sumy-K2*U2sumx)/(n);
        U20 = b2;
    }
}
void calibration::clearCalibration()
{
    U1sumx = 0;
    U1sumy = 0;
    U1sumxy = 0;
    U1sumx2 = 0;
    U2sumx = 0;
    U2sumy = 0;
    U2sumxy = 0;
    U2sumx2 = 0;
    K1 = 0;
    b1 = 0;
    K2 = 0;
    b2 = 0;
    n = 0;
}
float calibration::getK1()
{
    return 1/K1;
}
float calibration::getb1()
{
    return b1;
}
float calibration::getK2()
{
    return 1/K2;
}
float calibration::getb2()
{
    return b2;
}
float calibration::getU10()
{
    return U10;
}
float calibration::getU20()
{
    return U20;
}
float calibration::getU1h(float Pmax)
{
    U1h = (Pmax*K1)+U10;
    return U1h;
}
float calibration::getU2h(float Pmax)
{
    U2h = (Pmax*K2)+U20;
    return U2h;
}

