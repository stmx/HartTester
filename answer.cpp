#include "answer.h"

answer::answer(int len)
{
    requestLength = len;
    longFrame = 0;
    str = new unsigned char[len];
    for(int i =0; i<len;i++){
        str[i]=0;
    }
}
void answer::createAnswer(char *answer,int len){
    for(int i =0; i<len;i++){
        str[i]=answer[i];
    }
}

void answer::analysis(){
    int cursor = 0;
    preambleLength = 0;
    while(str[preambleLength] == 0xff){
        preambleLength++;
    }
    if(str[preambleLength] == 0x82){
        longFrame = true;
    }
    else if (str[preambleLength] == 0x02) {
        longFrame = false;
    }
    if(longFrame){
        int i = 0;
        addr = new char[5];
        while (i<5) {
            addr[i] = str[preambleLength+1+i];
            i++;
        }
        cursor = preambleLength+6;
    }
    else{
        addr = new char[1];
        addr[0] = str[preambleLength+1];
        cursor = preambleLength+2;
    }
    command = str[cursor];
    nDataByte = str[cursor+1];
    status[0] = str[cursor+2];
    status[1] = str[cursor+3];
    data = new char[nDataByte];
    int i = 0;
    while (i<nDataByte) {
        data[i] = str[cursor+4+i];
        i++;
    }
    Crc = str[cursor+4+nDataByte];
    requestLength = cursor+4+nDataByte+1;
}


char answer::CrcHart(char* dat, int len){
    char CrcHighReg=0,CrcLowReg=0;
    for(int c = 0; c<len-1 ; c++){
        CrcHighReg^= dat[c]&0xf0;
        CrcLowReg^= dat[c]&0x0f;
    }
    char t = CrcHighReg|CrcLowReg;
    return t;
}

char answer::createCrc(){
    int length;
    length = requestLength-preambleLength;
    char *str2 = new char[length];
    for(int i = 0; i<length; i++){
        str2[i] = str[i+preambleLength];
    }
    char t = CrcHart(str2,length);
    return t;
}

bool answer::CrcIsCorrect(){
    if(createCrc() == char(Crc)){
        return true;
    }
    return false;
}

int answer::getPreambleLength(){
    return preambleLength;
}
bool answer::isLongFrame(){
    return longFrame;
}
char* answer::getAddress(){
    return addr;
}
int answer::getCommand(){
    return command;
}
int answer::getnDataByte(){
    return nDataByte;
}
char* answer::getStatus(){
    return status;
}
char* answer::getData(){
    return data;
}
int answer::getCrc(){
    return Crc;
}
