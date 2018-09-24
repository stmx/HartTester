#include "request.h"

request::request()
{
    cursor = 0;
    for(int i = 0; i<50 ; i++){
        str[i] = 0;
    }
}
request::~request(){
    delete[] str;
}
void request::setPreambleLength(int a){
    preambleLength = a;
}
void request::setLongFrame(bool b){
    LongFrame = b;
}
void request::setAddress(unsigned char* addr){
    if (!LongFrame){
        address[0] = addr[0];
    }else {
        address[0] = addr[0];
        address[1] = addr[1];
        address[2] = addr[2];
        address[3] = addr[3];
        address[4] = addr[4];
    }
}
void request::createPreamble(){
    for(int i = 0;i < preambleLength;i++){
        str[i] = 0xff;
    }
    cursor = preambleLength;
}
void request::createStartByte(){
    if(!LongFrame){
        str[cursor] = 0x02;
        cursor++;
    }else {
        str[cursor] = 0x82;
        cursor++;
    }
}
void request::createAddress(){
    if (!LongFrame){
        str[cursor] = address[0];
        cursor++;
    }else {
        str[cursor+0] = address[0];
        str[cursor+1] = address[1];
        str[cursor+2] = address[2];
        str[cursor+3] = address[3];
        str[cursor+4] = address[4];
        cursor += 5;
    }
}
void request::createCommand(unsigned char c){
    str[cursor] = c;
    cursor++;
}
void request::createNumberDataByte(unsigned char d){
    str[cursor] = d;
    cursor++;
}
void request::createData(unsigned char *f,int NumberDataByte){
    int i = 0;
    while(i<NumberDataByte){
        str[cursor] = f[i];
        cursor++;
        i++;
    }
}
void request::createCrc(){
    int length;
    length = cursor-preambleLength;
    unsigned char *str2 = new unsigned char[length];
    for(int i = 0; i<length; i++){
        str2[i] = str[i+preambleLength];
    }
    str[cursor] = CrcHart(str2,length);
    requestLength = cursor+1;
}
unsigned char request::CrcHart(unsigned char* dat, int len){
    unsigned char CrcHighReg=0,CrcLowReg=0;
    for(int c = 0; c<len ; c++){
        CrcHighReg^= dat[c]&0xf0;
        CrcLowReg^= dat[c]&0x0f;
    }
    return CrcHighReg|CrcLowReg;
}
int request::getRequestLength(){
    return  requestLength;
}
int request::getPreambleLength(){
    return preambleLength;
}
char* request::getRequest(){
    return str;
}
void request::function0(){
    const char command = 0x00;
    const char NumberDataByte = 0x00;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    createCrc();
}
void request::function3(){
    const char command = 0x03;
    const char NumberDataByte = 0x00;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    createCrc();
}
void request::function13(){
    const char command = 0x0d;
    const char NumberDataByte = 0x02;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function35(){
    const char command = 0x23;
    const char NumberDataByte = 0x09;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function36(){
    const char command = 0x24;
    const char NumberDataByte = 0x00;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function37(){
    const char command = 0x25;
    const char NumberDataByte = 0x00;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function43(){
    const char command = 0x2b;
    const char NumberDataByte = 0x00;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function45(){
    const char command = 0x2d;
    const char NumberDataByte = 0x04;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    //createData(data,int(NumberDataByte));
    createCrc();
}
void request::function51(unsigned char *data){
    const char command = 0x33;
    const char NumberDataByte = 0x02;
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    createData(data,int(NumberDataByte));
    createCrc();
}
void request::function91(unsigned char *ndata,int nDataByte){
    const char command = 0x5b;
    const char NumberDataByte = char(nDataByte);
    createPreamble();
    createStartByte();
    createAddress();
    createCommand(command);
    createNumberDataByte(NumberDataByte);
    createData(ndata,int(NumberDataByte));
    createCrc();
}
