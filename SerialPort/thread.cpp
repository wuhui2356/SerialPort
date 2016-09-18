#include "thread.h"

Thread::Thread()
{
    m_stopFlag = false;
}

Thread::~Thread()
{
    stop();
}

void Thread::stop()
{
    QMutexLocker locker(&mutex);
    m_stopFlag = true;
}

void Thread::run()
{
    m_stopFlag = false;
    char read_buf[1024];
    while(1){
        {
           QMutexLocker locker(&mutex);
           if(m_stopFlag)
               break;
        }
        bzero(read_buf, sizeof(read_buf));
        port->readSerialPort(5000, read_buf, sizeof(read_buf));
        // qDebug() << "read: " << read_buf;
        emit sendDateToRecvText(QString(read_buf));

    }
    m_stopFlag = false;
}
