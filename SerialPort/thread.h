#ifndef THREAD
#define THREAD

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include "serialport.h"

class Thread:public QThread
{
    Q_OBJECT
public:
    Thread();
    ~Thread();
    void stop();
    void setPort(void *p){ port = (SerialPort *)p; }
//    void setCallBack(void (*p)(void *, void *), void *tmpUi, void *port){ QMutexLocker locker(&mutex); cb = p; ui = tmpUi; this->port = port; }

private:
    bool m_stopFlag;
//    void (*cb)(void *tmpUi, void* port);
//    void *ui;
    SerialPort *port;
    QMutex mutex;

protected:
    void run();

signals:
    void sendDateToRecvText(QString str);
};

#endif // THREAD

