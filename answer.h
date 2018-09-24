#ifndef ANSWER_H
#define ANSWER_H


class answer
{
    unsigned char *str;
    int requestLength;
    int preambleLength;
    bool longFrame;
    char *addr;
    int command;
    int nDataByte;
    char status[2];
    char *data;
    int Crc;
public:
    answer(int len);
    void analysis();
    void createAnswer(char *answer,int len);
    char getSym(int i);
    int getPreambleLength();
    bool isLongFrame();
    char* getAddress();
    int getCommand();
    int getnDataByte();
    char* getStatus();
    char* getData();
    char CrcHart(char* dat, int len);
    char createCrc();
    int getCrc();
    bool CrcIsCorrect();
};

#endif // ANSWER_H
