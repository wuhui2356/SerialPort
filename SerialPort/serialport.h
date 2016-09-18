#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <termios.h>     //POSIX中断控制定义
#include <poll.h>
#include <iostream>

typedef struct DATABUFFER {
    unsigned char name[2];
    unsigned char value[2];
} Databuffer;

typedef struct RDATA{
	unsigned char Head;
	unsigned char HeadCom[3];
	unsigned char Laddr[8];
	unsigned char Saddr[2];
	Databuffer data[4];
	unsigned char CRC;
	unsigned char LastByte;
} DATA;

const int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600, B300,  };
const int name_arr[] = {115200, 57600,38400, 19200, 9600, 4800, 2400, 1200, 600, 300, 115200, 57600,38400, 19200, 9600, 4800, 2400, 1200, 600, 300, };

class SerialPort
{
public:
    SerialPort();
    int openSerialPort();
    void SetSpeed();
    int SetParity();
    void closeSerialPort();
    void initSet(std::string serialPortName, int speed, int databits, int stopbits, char parity);
    int readSerialPort(int times, char *read_buf, int len);               // 需要时间间隔, 回调函数（显示读到的数据）
    int writeSerialPort(std::string str);
    ~SerialPort();

private:
    struct pollfd *fds;

    char parity;
    int speed, databits, stopbits;
    std::string serialPortName;

};




#endif
