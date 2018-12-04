#ifndef CALIBRATION_H
#define CALIBRATION_H


class calibration
{
    float U1sumx;
    float U1sumy;
    float U1sumxy;
    float U1sumx2;
    float U2sumx;
    float U2sumy;
    float U2sumxy;
    float U2sumx2;
    float K1;
    float b1;
    float K2;
    float b2;
    float U1h;
    float U10;
    float U2h;
    float U20;
    int n;
public:
    calibration();
    void addLine(float *b);
    void clearCalibration();
    float getK1();
    float getb1();
    float getK2();
    float getb2();
    float getU10();
    float getU20();
    float getU1h(float Pmax);
    float getU2h(float Pmax);
};

#endif // CALIBRATION_H
