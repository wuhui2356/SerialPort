#include <stdio.h>         //标准输入输出定义
#include <stdlib.h>        //标准函数库定义
#include <unistd.h>       //Unix标准函数定义
#include <errno.h>        //错误号定义
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <QDebug>
#include "serialport.h"

SerialPort::SerialPort()
{
    parity = 'N';
    speed = 9600;
    databits = 8;
    stopbits = 1;
    fds = new struct pollfd;
    memset(fds, 0, sizeof(struct pollfd));
}

SerialPort::~SerialPort()
{
    if(fds->fd)
    {
        closeSerialPort();
    }
    delete fds;
}

void SerialPort::closeSerialPort()
{
    close(fds->fd);
}

void SerialPort::initSet(std::string serialPortName, int speed, int databits, int stopbits, char parity)
{
    this->serialPortName = serialPortName;
    this->speed = speed;
    this->databits = databits;
    this->stopbits = stopbits;
    this->parity = parity;
}

//　如果运行该函数的线程结束时，readFlag　为true，则表示有错误发生
int SerialPort::readSerialPort(int times, char *read_buf, int len)
{
    int ret = 0;
    int readLen = 0;

    ret = poll(fds, 1, times);
    if(-1 == ret)
    {
        return ret;// 错误
    }
    else
    {
        usleep(500);
        readLen = read(fds->fd, read_buf, len - 1);
        return readLen;
    }
}

int SerialPort::writeSerialPort(std::string str)
{
    return write(fds->fd, str.c_str(), str.length());
}

int SerialPort::openSerialPort()
{
    fds->fd = open(serialPortName.c_str(), O_RDWR | O_NONBLOCK);    // 读写阻塞方式打开
    if(-1 != fds->fd)
    {
        fds->events = POLLIN;
    }
    return fds->fd;
}

void SerialPort::SetSpeed()
{
    int i;
    struct termios Opt;    //定义termios结构
    if(tcgetattr(fds->fd, &Opt) != 0)
    {
        perror("tcgetattr fds->fd");
        return;
    }
    for(i = 0; i < sizeof(speed_arr) / sizeof(int); i++)
    {
        if(speed == name_arr[i])
        {
            tcflush(fds->fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            if(tcsetattr(fds->fd, TCSANOW, &Opt) != 0)
            {
                perror("tcsetattr fds->fd");
                return;
            }
            tcflush(fds->fd, TCIOFLUSH);
        }
    }
}

int SerialPort::SetParity()
{
    struct termios Opt;
    if(tcgetattr(fds->fd, &Opt) != 0)
    {
        perror("tcgetattr fds->fd");
        return false;
    }
   Opt.c_cflag |= (CLOCAL | CREAD);        //一般必设置的标志

    switch(databits)        //设置数据位数
    {
    case 7:
        Opt.c_cflag &= ~CSIZE;
        Opt.c_cflag |= CS7;
        break;
    case 8:
        Opt.c_cflag &= ~CSIZE;
        Opt.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size.\n");
        return false;
    }

    switch(parity)            //设置校验位
    {
    case 'n':
    case 'N':
        Opt.c_cflag &= ~PARENB;        //清除校验位
        Opt.c_iflag &= ~INPCK;        //enable parity checking
        break;
    case 'o':
    case 'O':
        Opt.c_cflag |= PARENB;        //enable parity
        Opt.c_cflag |= PARODD;        //奇校验
        Opt.c_iflag |= INPCK;            //disable parity checking
        break;
    case 'e':
    case 'E':
        Opt.c_cflag |= PARENB;        //enable parity
        Opt.c_cflag &= ~PARODD;        //偶校验
        Opt.c_iflag |= INPCK;            //disable pairty checking
        break;
    case 's':
    case 'S':
        Opt.c_cflag &= ~PARENB;        //清除校验位
        Opt.c_cflag &= ~CSTOPB;        //??????????????
        Opt.c_iflag |= INPCK;            //disable pairty checking
        break;
    default:
        fprintf(stderr, "Unsupported parity.\n");
        return false;    
    }

    switch(stopbits)        //设置停止位
    {
    case 1:
        Opt.c_cflag &= ~CSTOPB;
        break;
    case 2:
        Opt.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stopbits.\n");
        return false;
    }

    Opt.c_cflag |= (CLOCAL | CREAD);

    Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
 
    Opt.c_oflag &= ~OPOST;
    Opt.c_oflag &= ~(ONLCR | OCRNL);    //添加的
 
    Opt.c_iflag &= ~(ICRNL | INLCR);
    Opt.c_iflag &= ~(IXON | IXOFF | IXANY);    //添加的

    tcflush(fds->fd, TCIFLUSH);
    Opt.c_cc[VTIME] = 0;        //设置超时为15sec
    Opt.c_cc[VMIN] = 0;        //Update the Opt and do it now
    if(tcsetattr(fds->fd, TCSANOW, &Opt) != 0)
    {
        perror("tcsetattr fds->fd");
        return false;
    }
    return true;
}


