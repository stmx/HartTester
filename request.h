#ifndef REQUEST_H
#define REQUEST_H

class request
{
    char *str = new char[50];
    unsigned char *address = new unsigned char[5];
    int preambleLength = 1;
    int cursor = 0;
    int requestLength;
    bool LongFrame = 0;
    void createPreamble();
    void createStartByte();
    void createAddress();
    void createCommand(unsigned char c);
    void createNumberDataByte(unsigned char d);
    void createData(unsigned char *f, int NumberDataByte);
    void createCrc();
    unsigned char CrcHart(unsigned char* dat, int len);
public:
    request();
    ~request();
    void function0();
    void function3();
    void function13();
    void function35();
    void function36();
    void function37();
    void function43();
    void function45();
    void function51(unsigned char *data);
    void function91(unsigned char *ndata,int nDataByte);
    void setPreambleLength(int a);
    void setLongFrame(bool b);
    void setAddress(unsigned char* addr);
    int getRequestLength();
    int getPreambleLength();
    char* getRequest();
};

#endif // REQUEST_H
