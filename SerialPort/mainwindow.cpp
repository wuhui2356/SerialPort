#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <errno.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openFlag = false;
    timerFlag = false;
    autosendFlag = false;
    recvNum = 0;
    sendNum = 0;

    seriport = new SerialPort;
    thread = new Thread;
    timer = new QTimer;
    label = new QLabel;
    systemLabel = new QLabel;
    systemTimer = new QTimer;
    sendLable = new QLabel;
    recvLable = new QLabel;

    systemTimer->setInterval(1000);

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeOutEvent()));
    QObject::connect(systemTimer, SIGNAL(timeout()), this, SLOT(systemTimeUpdate()));
    QObject::connect(thread, SIGNAL(sendDateToRecvText(QString)), this, SLOT(showInRecvText(QString)));
    QObject::connect(ui->recvTextEdit, SIGNAL(textChanged()), this, SLOT(autoScroll()));
    QObject::connect(ui->rateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reLoadSerialPort()));
    QObject::connect(ui->checkBitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reLoadSerialPort()));
    QObject::connect(ui->dataBitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reLoadSerialPort()));
    QObject::connect(ui->stopBitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reLoadSerialPort()));

    QObject::connect(ui->sendLineEdit3, SIGNAL(returnPressed()), this, SLOT(sendLineEdit3()));
    QObject::connect(ui->sendPushButton3, SIGNAL(clicked(bool)), this, SLOT(sendLineEdit3()));
    QObject::connect(ui->sendLineEdit4, SIGNAL(returnPressed()), this, SLOT(sendLineEdit4()));
    QObject::connect(ui->sendPushButton4, SIGNAL(clicked(bool)), this, SLOT(sendLineEdit4()));
    QObject::connect(ui->sendLineEdit5, SIGNAL(returnPressed()), this, SLOT(sendLineEdit5()));
    QObject::connect(ui->sendPushButton5, SIGNAL(clicked(bool)), this, SLOT(sendLineEdit5()));
    QObject::connect(ui->sendLineEdit6, SIGNAL(returnPressed()), this, SLOT(sendLineEdit6()));
    QObject::connect(ui->sendPushButton6, SIGNAL(clicked(bool)), this, SLOT(sendLineEdit6()));
    SerialPortInit();

    label->setText(QString::fromUtf8("     串口状态：　关闭"));
    ui->statusBar->addWidget(label);

    recvLable->setText(QString::fromUtf8("\t接收：") + QString::number(recvNum));
    ui->statusBar->addWidget(recvLable);

    sendLable->setText(QString::fromUtf8("\t发送：") + QString::number(sendNum));
    ui->statusBar->addWidget(sendLable);

    systemLabel->setText(QString::fromUtf8("\t当前时间：") + QTime::currentTime().toString());
    ui->statusBar->addWidget(systemLabel);

    systemTimer->start();
}

MainWindow::~MainWindow()
{
    seriport->closeSerialPort();
    systemTimer->stop();
    delete ui;
    delete thread;
    delete seriport;
    delete timer;
    delete label;
    delete systemLabel;
    delete systemTimer;
    delete recvLable;
    delete sendLable;
}

void MainWindow::systemTimeUpdate()
{
    recvLable->setText(QString::fromUtf8("\t接收：") + QString::number(recvNum));
    sendLable->setText(QString::fromUtf8("\t发送：") + QString::number(sendNum));
    systemLabel->setText(QString::fromUtf8("\t当前时间：") + QTime::currentTime().toString());
}

void MainWindow::reLoadSerialPort()
{
    if(openFlag)
    {
        on_openPushButton_clicked();
    }
}

// 接收区自动滚屏
void MainWindow::autoScroll()
{
    QTextCursor cursor =  ui->recvTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->recvTextEdit->setTextCursor(cursor);
}

// 显示收到的数据的槽
void MainWindow::showInRecvText(QString str)
{
    recvNum += str.length();
    if(ui->hexShowCheckBox->isChecked())
    {
        bool ok = true;
        QString result = QString::number(str.toInt(&ok, 10),16);
        if(ok)
        {
            ui->recvTextEdit->insertPlainText(result.toUtf8());
        }
        else
        {
            result = "";
            for(int i = 0; i < str.length(); ++i)
            {
                result += QString::number(str[i].toAscii());
            }
            ui->recvTextEdit->insertPlainText(result.toUtf8());
        }
    }
    else
    {
        ui->recvTextEdit->insertPlainText(str.toUtf8());
    }
}

//  初始化槽，用于动态加载当前的串口到combobox
void MainWindow::SerialPortInitSlots()
{
    SerialPortInit();
}

//　初始化函数，获取当前的所有串口
void MainWindow::SerialPortInit()
{
    QFile file(PORTFILENAME);
    if(file.exists())       // 如果文件存在删除，或清空
    {
        file.remove();
    }

    // 查看当前的串口，并保存到文件中
    int ret = system("ls /dev/ttyUSB* > serialport.dat");
    if(ret < 0)
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("串口异常"));
    }

    //　读取文件中的串口并添加到combobox中
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("串口异常"));
        exit(0);
    }

    // 读取当前的串口并添加到combobox中
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        line.remove(0, 5);
        ui->serialPortComboBox->addItem(QString::fromLocal8Bit(line).trimmed());
    }

    file.close();
    file.remove();
}

// 打开串口，且开启线程读取串口
void MainWindow::on_openPushButton_clicked()
{
    if(!openFlag)
    {
        QString serialPortStr = ui->serialPortComboBox->currentText().isEmpty() ? "" : ui->serialPortComboBox->currentText();
        QString rateStr = ui->rateComboBox->currentText().isEmpty() ? "" : ui->rateComboBox->currentText();
        QString checkBitStr = ui->checkBitComboBox->currentText().isEmpty() ? "" : ui->checkBitComboBox->currentText();
        QString dataBirStr = ui->dataBitComboBox->currentText().isEmpty() ? "" : ui->dataBitComboBox->currentText();
        QString stopBbitStr = ui->stopBitComboBox->currentText().isEmpty() ? "" : ui->stopBitComboBox->currentText();

        qDebug() << "port: " << serialPortStr;
        qDebug() << "rate: " << rateStr;
        qDebug() << "check: " << checkBitStr;
        qDebug() << "data: " << dataBirStr;
        qDebug() << "stop: " << stopBbitStr;

        if(serialPortStr == "")
        {
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("未找到串口"));
            return;
        }

        if(!rateStr.isNull() && !checkBitStr.isNull() && !dataBirStr.isNull() && !stopBbitStr.isNull())
        {
            bool rateOK = false;
            bool dataOK = false;
            bool stopOK = false;
            serialPortStr = "/dev/" + serialPortStr;
            seriport->initSet(serialPortStr.toStdString(), rateStr.toInt(&rateOK), dataBirStr.toInt(&dataOK), stopBbitStr.toInt(&stopOK), checkBitStr.toStdString()[0]);
            if(!rateOK || !dataOK || !stopOK)
            {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("串口异常"));
                exit(0);
            }
        }
        else
        {
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("串口异常"));
            exit(0);
        }
        int ret = seriport->openSerialPort();
        qDebug() << "open ret: " << ret;
        if(-1 == ret)
        {
            perror("open error");
            qDebug() << "open error errno: " <<  errno;
            if(errno == 13)
            {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请使用root权限"));
            }
            else
            {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("串口异常"));
            }
            seriport->closeSerialPort();
        }
        else
        {
            seriport->SetSpeed();
            seriport->SetParity();
            ui->openPushButton->setText(QString::fromUtf8("关闭串口"));
            openFlag = true;
            label->setText(QString::fromUtf8("     串口状态：　打开"));
            sendNum = 0;
            recvNum = 0;
            thread->setPort((void *)seriport);
            thread->start();
        }
    }
    else    // 关闭串口
    {
        seriport->closeSerialPort();
        ui->openPushButton->setText(QString::fromUtf8("打开串口"));
        openFlag = false;
        label->setText(QString::fromUtf8("     串口状态：　关闭"));
        thread->stop();
        autosendFlag = false;
        qDebug() << "closed ...";
    }
}

// 清空接收区
void MainWindow::on_clearRecvPushButton_clicked()
{
    ui->recvTextEdit->clear();
}

// 发送１，不清空发送区
void MainWindow::on_sendPushButton1_clicked()
{
    if(openFlag)
    {
        bool ok = true;
        QString sendStr;
        if(ui->hexSendcheckBox->isChecked())
        {
            sendStr = QString::number(ui->sendTextEdit1->toPlainText().toInt(&ok, 16));
        }
        else
        {
            sendStr = ui->sendTextEdit1->toPlainText();
        }
        if(ok)
        {
            sendNum += sendStr.length();
            seriport->writeSerialPort(sendStr.toStdString());
            ui->recvTextEdit->insertPlainText("send: " + sendStr + '\n');
        }
        else
        {
            QString result;
            sendStr = ui->sendTextEdit1->toPlainText();
            for(int i = 0; i < sendStr.length(); ++i)
            {
                QString tmpStr = QString::number(sendStr[i].toAscii(), 16);
                result += tmpStr;
            }
            sendNum += sendStr.length();
            seriport->writeSerialPort(result.toStdString());
            ui->recvTextEdit->insertPlainText("send: " + result + '\n');
        }
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

// 发送２，要判断是否自动清空发送区２
void MainWindow::on_sendPushButton2_clicked()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit2->text();
        sendNum += sendStr.length();
        seriport->writeSerialPort(sendStr.toStdString());
        if(ui->autoClearSendcheckBox->isChecked())
        {
            ui->sendLineEdit2->clear();
        }
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

// 发送２，要判断是否自动清空发送区２,按回车建时发送数据，要发送回车
void MainWindow::on_sendLineEdit2_returnPressed()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit2->text();
        sendNum += sendStr.length() + 1;
        seriport->writeSerialPort(sendStr.toStdString() + '\n');
        if(ui->autoClearSendcheckBox->isChecked())
        {
            ui->sendLineEdit2->clear();
        }
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

// 发送３
void MainWindow::sendLineEdit3()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit3->text();
        sendNum += sendStr.length();
        seriport->writeSerialPort(sendStr.toStdString());
        ui->recvTextEdit->insertPlainText("send: " + sendStr + '\n');
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

// 发送4
void MainWindow::sendLineEdit4()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit4->text();
        sendNum += sendStr.length();
        seriport->writeSerialPort(sendStr.toStdString());
        ui->recvTextEdit->insertPlainText("send: " + sendStr + '\n');
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}


// 发送5
void MainWindow::sendLineEdit5()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit5->text();
        sendNum += sendStr.length();
        seriport->writeSerialPort(sendStr.toStdString());
        ui->recvTextEdit->insertPlainText("send: " + sendStr + '\n');
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}


// 发送6
void MainWindow::sendLineEdit6()
{
    if(openFlag)
    {
        QString sendStr = ui->sendLineEdit6->text();
        sendNum += sendStr.length();
        seriport->writeSerialPort(sendStr.toStdString());
        ui->recvTextEdit->insertPlainText("send: " + sendStr + '\n');
    }
    else
    {
        QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
    }
}

void MainWindow::timeOutEvent()
{
    qDebug() << "time out ... ";
    on_sendPushButton1_clicked();
}

void MainWindow::on_autoSendCheckBox_clicked()
{
    if(!autosendFlag)   // start
    {
        if(openFlag)
        {
            bool ok = true;
            int num = ui->autoSendPeriodLineEdit->text().toInt(&ok);
            if(ok)
            {
                timer->setInterval(num);
            }
            else
            {
                QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("时间间隔错误"));
                return;
            }
            timer->start();
            openFlag = true;
            autosendFlag = true;
            qDebug() << "timer start ...";
        }
        else
        {
            ui->autoSendCheckBox->setCheckable(false);
            QMessageBox::information(this, QString::fromUtf8("提示"), QString::fromUtf8("请打开串口"));
        }
    }
    else    // stop
    {
        timer->stop();
        qDebug() << "timer stop ...";
        autosendFlag = false;
    }
}
