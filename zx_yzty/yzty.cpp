#include "yzty.h"
#include "ui_yzty.h"
#include "crc.h"
#include <QDateTime>
#include <QElapsedTimer>
#include <QTextStream>
#include <QSettings>
#include "ymodem.h"

YZTY::YZTY(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YZTY)
{
    ui->setupUi(this);
    ui_init();
    isConnetOK  = false;
    net_init();
    com_init();
    timer_init();
    flag_auto_send_yc      = false;
    flag_16_ts             = true;
    flag_auto_get_master_t = false;
    flag_auto_send_heart   = false;

    flag_program           = false;
    flag_ymodem_start      = false;

    timeoutHand            = 0;
    timeoutAuto            = 0;
    sendCode               = 0;
    sendAutoCode           = 0;
    rebootTime             = 1;
    line                   = 0;
    reboot_reason          = 0;

    ymodem_timeout         = 0;
    ymodem_packet_counter  = 0;
    ymodem_packet_number   = 0;
    ymodem_last_send       = 0;
    ymodem_last_rec        = 0;

    ui->checkBox_16_ts->setChecked(true);
}

YZTY::~YZTY()
{
    delete ui;
}

void YZTY::ui_init()
{
    ui->stackedWidget_nav->setCurrentIndex(0);
    ui->ty_display->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ty_display->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->ala_display->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->ala_display->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch);
    ui->ala_display->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);
    ui->uart_conn_stat->setEnabled(false);
    ui->dev_stat->setEnabled(false);
    ui->dw_stat->setEnabled(false);
    ui->ty_mode_stat->setEnabled(false);
    ui->alm_num_stat->setEnabled(false);
    ui->ty_num_stat->setEnabled(false);

    ui->dev_s_version->setEnabled(false);
    ui->dev_h_version->setEnabled(false);

    ui->master_time->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->progressBar->setValue(0);
    ui->program_stat->setEnabled(false);
    QStringList ty_mode_select_list;
    ty_mode_select_list<<QString::fromLocal8Bit("自动")<<QString::fromLocal8Bit("手动");
    ui->ty_mode_select->addItems(ty_mode_select_list);
    QStringList yk_list;
    yk_list<<QString::fromLocal8Bit("升压")<<QString::fromLocal8Bit("降压")<<QString::fromLocal8Bit("调容");
    ui->yk_mode_select->addItems(yk_list);
    QStringList yk_enter_list;
    yk_enter_list<<QString::fromLocal8Bit("确认")<<QString::fromLocal8Bit("取消");
    ui->yk_enter_mode_select->addItems(yk_enter_list);
    QStringList clr_rec_list;
    clr_rec_list<<QString::fromLocal8Bit("调压记录")<<QString::fromLocal8Bit("告警记录")<<QString::fromLocal8Bit("重启记录");
    ui->clear_rec_select->addItems(clr_rec_list);
    QStringList save_rec_list;
    save_rec_list<<QString::fromLocal8Bit("调压记录")<<QString::fromLocal8Bit("告警记录");
    ui->save_rec_select->addItems(save_rec_list);
    ui->tran_switch_total->setEnabled(false);

    ui->radioButton_uart->setChecked(0);
    ui->radioButton_net->setChecked(1);
}
void YZTY::on_radioButton_net_clicked(bool stat)
{
    if(stat == true)
    {
        ui->radioButton_net->setChecked(1);
        ui->pushButton_conn_net->setEnabled(true);
        ui->label_net_stat->setEnabled(true);
        ui->net_conn_stat->setEnabled(true);
        ui->radioButton_uart->setChecked(0);
        ui->pushButton_conn_com->setEnabled(false);
        ui->label_uart_stat->setEnabled(false);
        ui->uart_conn_stat->setEnabled(false);
    }
}
void YZTY::on_radioButton_uart_clicked(bool stat)
{
    if(stat == true)
    {
        ui->radioButton_net->setChecked(0);
        ui->pushButton_conn_net->setEnabled(false);
        ui->label_net_stat->setEnabled(false);
        ui->net_conn_stat->setEnabled(false);
        ui->radioButton_uart->setChecked(1);
        ui->pushButton_conn_com->setEnabled(true);
        ui->label_uart_stat->setEnabled(true);
        ui->uart_conn_stat->setEnabled(true);
        com_init();
    }
}
void YZTY::net_init()
{
    //初始化TCP客户端
    tcpClient = new QTcpSocket(this);   //实例化tcpClient
}
void YZTY::com_init()
{
    mycom      = new QSerialPort();
    QSerialPortInfo *port_info = new QSerialPortInfo(ui->comboBox_num_com->currentText());
    mycom->setPort(*port_info);
    readCount  = 0;
    revLength  = 0;
    //循环查找串口
    foreach(const QSerialPortInfo &Info, QSerialPortInfo::availablePorts())
    {
        QSerialPort availablePort;
        availablePort.setPortName(Info.portName());
        if (availablePort.open(QIODevice::ReadWrite))
        {
            ui->comboBox_num_com->findText(Info.portName());
            ui->comboBox_num_com->addItem(Info.portName());
            availablePort.close();  //打开后关闭
        }
    }
    QStringList baud_list;
    baud_list<<"9600"<<"19200"<<"38400"<<"115200";
    ui->comboBox_baud_com->addItems(baud_list);
    QStringList flow_list;
    flow_list<<QString::fromLocal8Bit("无流控制")<<QString::fromLocal8Bit("硬件流控制")<<QString::fromLocal8Bit("软件流控制");
    ui->comboBox_flow_com->addItems(flow_list);
    QStringList check_list;
    check_list<<QString::fromLocal8Bit("无校验")<<QString::fromLocal8Bit("偶校验")<<QString::fromLocal8Bit("奇校验");
    ui->comboBox_check_com->addItems(check_list);
    QStringList stop_list;
    stop_list<<"1"<<"1.5"<<"2";
    ui->comboBox_stop_com->addItems(stop_list);
    QStringList data_list;
    data_list<<"8"<<"5"<<"6"<<"7";
    ui->comboBox_data_com->addItems(data_list);
}
void YZTY::timer_init()
{
    mainTimer          = new QTimer(this);
    programTimer       = new QTimer(this);
    connect(mainTimer,SIGNAL(timeout()),this,SLOT(mainTimer_update()));
    connect(programTimer,SIGNAL(timeout()),this,SLOT(programTimer_update()));
}
void YZTY::on_listWidget_nav_itemClicked()
{
   int row = ui->listWidget_nav->currentRow();
   switch (row)
   {
   case 0:
       ui->stackedWidget_nav->setCurrentIndex(0);
       break;
   case 1:
       ui->stackedWidget_nav->setCurrentIndex(1);
       break;
   case 2:
       ui->stackedWidget_nav->setCurrentIndex(2);
       break;
   case 3:
       ui->stackedWidget_nav->setCurrentIndex(3);
       break;
   case 4:
       ui->stackedWidget_nav->setCurrentIndex(4);
       break;
   case 5:
       ui->stackedWidget_nav->setCurrentIndex(5);
       break;
   case 6:
       ui->stackedWidget_nav->setCurrentIndex(6);
       break;
   case 7:
       ui->stackedWidget_nav->setCurrentIndex(7);
       break;
   case 8:
       ui->stackedWidget_nav->setCurrentIndex(8);
       break;
   default:
       break;
   }
}
void YZTY::displayError(QAbstractSocket::SocketError)
{
    ui->textEdit_print_ts->setText(tcpClient->errorString());
    tcpClient->close();
}
void YZTY::on_pushButton_conn_net_clicked()
{
    if(isConnetOK == false)
    {
        tcpClient->abort();                 //取消原有连接
        connect(tcpClient, SIGNAL(readyRead()), this, SLOT(read_data()));
        connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
        tcpClient->connectToHost(ui->lineEdit_server_ip->text(), ui->lineEdit_server_port->text().toInt());
        if (!tcpClient->waitForConnected(1000))  // 连接成功则进入
        {
            QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("连接网络失败！"));
        }
        else
        {
            isConnetOK = true;
            mainTimer->start(100);
            ui->net_conn_stat->setText(QString::fromLocal8Bit("已连接"));
            ui->pushButton_conn_net->setText(QString::fromLocal8Bit("断开网络"));
            ui->radioButton_net->setEnabled(false);
            ui->radioButton_uart->setEnabled(false);
        }
    }
    else if(isConnetOK == true)
    {
        tcpClient->disconnectFromHost();
        if (tcpClient->state() == QAbstractSocket::UnconnectedState ||
                  tcpClient->waitForDisconnected(1000))
        {
            tcpClient->close();
            disconnect(tcpClient,SIGNAL(readyRead()),0,0);
            disconnect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),0,0);
            mainTimer->stop();
            ui->net_conn_stat->setText(QString::fromLocal8Bit("已断开"));
            ui->pushButton_conn_net->setText(QString::fromLocal8Bit("连接网络"));
            ui->radioButton_net->setEnabled(true);
            ui->radioButton_uart->setEnabled(true);
            isConnetOK = false;
        }
        else
        {
            QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("断开网络失败！"));
        }
    }
}
void YZTY::on_pushButton_conn_com_clicked()
{
    if(isConnetOK == false)
    {
        connect(mycom,SIGNAL(readyRead()),this,SLOT(read_data()),Qt::UniqueConnection);

        int select = 0;
        select = ui->comboBox_baud_com->currentText().toInt();
        switch(select)
        {
        case 9600:
            mycom->setBaudRate(QSerialPort::Baud9600);
            break;
        case 19200:
            mycom->setBaudRate(QSerialPort::Baud19200);
            break;
        case 38400:
            mycom->setBaudRate(QSerialPort::Baud38400);
            break;
        case 115200:
            mycom->setBaudRate(QSerialPort::Baud115200);
            break;
        default:
            mycom->setBaudRate(QSerialPort::Baud9600);
            break;
        }
        select = ui->comboBox_check_com->currentIndex();
        switch(select)
        {
        case 0:
            mycom->setParity(QSerialPort::NoParity);
            break;
        case 1:
            mycom->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            mycom->setParity(QSerialPort::OddParity);
            break;
        default:
            mycom->setParity(QSerialPort::NoParity);
            break;
        }
        select = ui->comboBox_data_com->currentText().toInt();
        switch(select)
        {
        case 8:
            mycom->setDataBits(QSerialPort::Data8);
            break;
        case 5:
            mycom->setDataBits(QSerialPort::Data5);;
            break;
        case 6:
            mycom->setDataBits(QSerialPort::Data6);;
            break;
        case 7:
            mycom->setDataBits(QSerialPort::Data7);;
            break;
        default:
            mycom->setDataBits(QSerialPort::Data8);
            break;
        }
        select = ui->comboBox_stop_com->currentIndex();
        switch(select)
        {
        case 1:
            mycom->setStopBits(QSerialPort::OneStop);
            break;
        case 2:
            mycom->setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 3:
            mycom->setStopBits(QSerialPort::TwoStop);
            break;
        default:
            mycom->setStopBits(QSerialPort::OneStop);
            break;
        }
        select = ui->comboBox_flow_com->currentText().toInt();
        switch(select)
        {
        case 0:
            mycom->setFlowControl(QSerialPort::NoFlowControl);
            break;
        case 1:
            mycom->setFlowControl(QSerialPort::HardwareControl);
            break;
        case 2:
            mycom->setFlowControl(QSerialPort::SoftwareControl);
            break;
        default:
            mycom->setFlowControl(QSerialPort::NoFlowControl);
            break;
        }
        if(!mycom->open(QIODevice::ReadWrite))
            QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("打开串口失败！"));
        else
        {
            isConnetOK = true;
            mainTimer->start(100);
            ui->uart_conn_stat->setText(QString::fromLocal8Bit("已打开"));
            ui->pushButton_conn_com->setText(QString::fromLocal8Bit("关闭串口"));

            ui->radioButton_net->setEnabled(false);
            ui->radioButton_uart->setEnabled(false);
        }
    }
    else if(isConnetOK == true)
    {
        mycom->clear();
        mycom->close();
        disconnect(mycom,SIGNAL(readyRead()),0,0);
        mainTimer->stop();
        ui->uart_conn_stat->setText(QString::fromLocal8Bit("已关闭"));
        ui->pushButton_conn_com->setText(QString::fromLocal8Bit("打开串口"));
        ui->radioButton_net->setEnabled(true);
        ui->radioButton_uart->setEnabled(true);
        isConnetOK = false;
    }
}
void YZTY::send_data(const char *sendData, int len)
{
    if(ui->radioButton_uart->isChecked() == true)
        mycom->write(sendData, len);
    else if(ui->radioButton_net->isChecked() == true)
        tcpClient->write(sendData, len);;
}
void YZTY::read_data()
{
    QByteArray  buf;
    if(ui->radioButton_uart->isChecked() == true)
        buf = mycom->readAll();
    else if(ui->radioButton_net->isChecked() == true)
        buf = tcpClient->readAll();
    if(buf.isEmpty())
        return;
    int i;
    QString s;

    /*---------------------程序下载部分---------------*/
    if(flag_program == true)
    {
        for(i = 0; i < buf.size(); i++)
        {
            readData[0] = buf[i];

            if(readData[0] == ACK)
                s = QString::fromLocal8Bit("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 设备回复：ACK");
            else if(readData[0] == CR)
                s = QString::fromLocal8Bit("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 设备回复：CR");
            else if(readData[0] == NAK)
                s = QString::fromLocal8Bit("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 设备回复：NAK");
            else if(readData[0] == CA)
                s = QString::fromLocal8Bit("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 设备回复：CA");
            else
                s=QString(readData[0]);

            ui->textEdit_print_ts->moveCursor(QTextCursor::End);
            ui->textEdit_print_ts->insertPlainText(s);
            if(flag_ymodem_start == false)
            {
                if(readData[0] == 'S')
                {
                    char data_u = 'U';
                    send_data(&data_u, 1);
                    s =  QString::fromLocal8Bit(" <<<<<<<<<<<<<<<<<<<<<<<<<<<设备启动标志\r\n主机发送：U >>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                    ui->textEdit_print_ts->insertPlainText(s);
                }
                else if(readData[0] == 'C')
                {
                    ymodem_send_SOH(0);
                    ymodem_last_send  = SOH;
                    ymodem_last_rec   = CR;
                    flag_ymodem_start = true;
                }
            }
            else if(flag_ymodem_start == true)
            {
                switch(readData[0])
                {
                case ACK:
                    program_handle(ACK);
                    ymodem_last_rec = ACK;
                    break;
                case CR:
                    program_handle(CR);
                    ymodem_last_rec = CR;
                    break;
                case NAK:
                    program_handle(NAK);
                    ymodem_last_rec = NAK;
                    break;
                case CA:
                    program_handle(CA);
                    ymodem_last_rec = CA;
                    break;
                }
            }
        }
        return;
    }
    /*---------------------------------------------*/
    for(i = 0; i < buf.size(); i++)
    {
        readData[readCount] = (unsigned char)buf[i];
        switch(readCount)
        {
        case 0:
            if(readData[readCount] == 0x55)
                readCount++;
            else
            {
                s=QString(readData[0]);
                ui->textEdit_print_ts->moveCursor(QTextCursor::End);
                ui->textEdit_print_ts->insertPlainText(s);
                readCount = 0;
            }
            break;
        case 1:
            if(readData[readCount] == 0xAA)
                readCount++;
            else
            {
                s=QString(readData[0]) + QString(readData[1]);
                ui->textEdit_print_ts->moveCursor(QTextCursor::End);
                ui->textEdit_print_ts->insertPlainText(s);
                readCount = 0;
            }
            break;
        case 2:
            if(readData[readCount] == DEVICE_ID)
                readCount++;
            else
                readCount = 0;
            break;
        case 3:
            readCount++;
            break;
        case 4:
            revLength = readData[readCount] << 8;
            readCount++;
            break;
        case 5:
            revLength = revLength +  readData[readCount];
            readCount++;
            break;
        default:
            if(readCount < revLength - 1)
                readCount++;
            else
                rev_data_handle();
            break;
        }
    }
}

void YZTY::heart_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("心跳命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    QString dangwei = QString::number(readData[6]);
    ui->dw_stat->setText(dangwei);
    if(readData[7] == 0)
        ui->ty_mode_stat->setText(QString::fromLocal8Bit("自动模式"));
    else if(readData[7] == 1)
        ui->ty_mode_stat->setText(QString::fromLocal8Bit("手动模式"));
    QString ty_num = QString::number((readData[8] << 24) + (readData[9] << 16) + (readData[10] << 8) + readData[11]);
    ui->ty_num_stat->setText(ty_num);
    QString alarm_num = QString::number((readData[12] << 8) + readData[13]);
    ui->alm_num_stat->setText(alarm_num);

    QString reboot_num = QString::number((readData[16] << 8) + readData[17]);
    ui->reboot_num_stat->setText(reboot_num);
    QString mem_dw =  QString::number(readData[18]);
    ui->mem_dw_stat->setText(mem_dw);

    if(readData[19] == 0)
        ui->cap_stat->setText(QString::fromLocal8Bit("大"));
    else if(readData[19] == 1)
        ui->cap_stat->setText(QString::fromLocal8Bit("小"));

    if(readData[14] != 0 || readData[15] != 0)
        ui->dev_stat->setText(QString::fromLocal8Bit("告警"));
    else
        ui->dev_stat->setText(QString::fromLocal8Bit("正常"));
    if((readData[15] & 0x01) == 0x01)
        ui->init_fail->setChecked(true);
    else
        ui->init_fail->setChecked(false);

    if((readData[15] & 0x02) == 0x02)
        ui->att7022_err->setChecked(true);
    else
        ui->att7022_err->setChecked(false);

    if((readData[15] & 0x04) == 0x04)
        ui->fram_err->setChecked(true);
    else
        ui->fram_err->setChecked(false);

    if((readData[15] & 0x08) == 0x08)
        ui->gear_fault->setChecked(true);
    else
        ui->gear_fault->setChecked(false);

    if((readData[15] & 0x10) == 0x10)
        ui->motor_fault->setChecked(true);
    else
        ui->motor_fault->setChecked(false);

    if((readData[15] & 0x20) == 0x20)
        ui->power_off->setChecked(true);
    else
        ui->power_off->setChecked(false);

    if((readData[15] & 0x40) == 0x40)
        ui->over_cur->setChecked(true);
    else
        ui->over_cur->setChecked(false);

    if((readData[15] & 0x80) == 0x80)
        ui->low_vol->setChecked(true);
    else
        ui->low_vol->setChecked(false);

    if((readData[14] & 0x01) == 0x01)
        ui->high_vol->setChecked(true);
    else
        ui->high_vol->setChecked(false);

    if((readData[14] & 0x02) == 0x02)
        ui->switch_lock->setChecked(true);
    else
        ui->switch_lock->setChecked(false);

    if((readData[14] & 0x04) == 0x04)
        ui->oil_pos_alarm->setChecked(true);
    else
        ui->oil_pos_alarm->setChecked(false);
    if((readData[14] & 0x08) == 0x08)
        ui->oil_tmp_alarm->setChecked(true);
    else
        ui->oil_tmp_alarm->setChecked(false);
    if((readData[14] & 0x10) == 0x10)
        ui->turn_vol_fail->setChecked(true);
    else
        ui->turn_vol_fail->setChecked(false);
    if((readData[14] & 0x20) == 0x10)
        ui->turn_cap_fail->setChecked(true);
    else
        ui->turn_cap_fail->setChecked(false);
}
void YZTY::on_hand_input_code_clicked()
{
    int alarmCode ;
    alarmCode =ui->lineEdit_hand_input_code->text().toInt();
    if((alarmCode & 0x01) == 0x01)
        ui->init_fail->setChecked(true);
    else
        ui->init_fail->setChecked(false);

    if((alarmCode & 0x02) == 0x02)
        ui->att7022_err->setChecked(true);
    else
        ui->att7022_err->setChecked(false);

    if((alarmCode & 0x04) == 0x04)
        ui->fram_err->setChecked(true);
    else
        ui->fram_err->setChecked(false);

    if((alarmCode & 0x08) == 0x08)
        ui->gear_fault->setChecked(true);
    else
        ui->gear_fault->setChecked(false);

    if((alarmCode & 0x10) == 0x10)
        ui->motor_fault->setChecked(true);
    else
        ui->motor_fault->setChecked(false);

    if((alarmCode & 0x20) == 0x20)
        ui->power_off->setChecked(true);
    else
        ui->power_off->setChecked(false);

    if((alarmCode & 0x40) == 0x40)
        ui->over_cur->setChecked(true);
    else
        ui->over_cur->setChecked(false);

    if((alarmCode & 0x80) == 0x80)
        ui->low_vol->setChecked(true);
    else
        ui->low_vol->setChecked(false);

    if((alarmCode & 0x100) == 0x100)
        ui->high_vol->setChecked(true);
    else
        ui->high_vol->setChecked(false);

    if((alarmCode & 0x200) == 0x200)
        ui->switch_lock->setChecked(true);
    else
        ui->switch_lock->setChecked(false);

    if((alarmCode & 0x400) == 0x400)
        ui->oil_pos_alarm->setChecked(true);
    else
        ui->oil_pos_alarm->setChecked(false);
    if((alarmCode & 0x800) == 0x800)
        ui->oil_tmp_alarm->setChecked(true);
    else
        ui->oil_tmp_alarm->setChecked(false);
}
void YZTY::print(int len, unsigned char* data, int error)
{
    QString s, s1;
    char chBuf[4];
    if(flag_16_ts == true)
    {
        for(int i = 0; i < len; i++)
        {
            sprintf(chBuf, "%02x ", data[i]);
            s1 = QString(QLatin1String(chBuf));
            s += s1;
         }
    }
    else
    {
        char *str1 = (char *)data;
        s = QString(QLatin1String(str1));
    }
    if(error == 0)
        ui->textEdit_print_ts->setTextColor(Qt::black);
    else if(error == 1)
        ui->textEdit_print_ts->setTextColor(Qt::red);
    ui->textEdit_print_ts->append(s);
}
void YZTY::on_pushButton_empty_ts_clicked()
{
    ui->textEdit_print_ts->clear();
}

void YZTY::on_checkBox_yc_switch_clicked()
{
    if(ui->checkBox_yc_switch->isChecked() == true)
    {
        flag_auto_send_yc = true;
    }
    else
        flag_auto_send_yc = false;
}
void YZTY::send_yc()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = YC_CODE;
    sendData[4] = SEND_YC_LEN >> 8;
    sendData[5] = SEND_YC_LEN & 0xFF;
    crc = CRC_16(sendData, SEND_YC_LEN - 2);
    sendData[6] = crc >> 8;
    sendData[7] = crc & 0xFF;
    send_data((const char *)sendData, SEND_YC_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送遥测命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_YC_LEN, sendData, 0);
}
void YZTY::on_get_yc_switch_clicked()
{
    sendCode = YC_CODE;
}
void YZTY::yc_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("遥测命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    float u_a = ((readData[6]<<8) + readData[7])/10.0;
    ui->lineEdit_u_a_yc->setText(QString::number(u_a));

    float u_b = ((readData[8]<<8) + readData[9])/10.0;
    ui->lineEdit_u_b_yc->setText(QString::number(u_b));

    float u_c = ((readData[10]<<8) + readData[11])/10.0;
    ui->lineEdit_u_c_yc->setText(QString::number(u_c));

    float i_a = ((readData[12]<<8) + readData[13])/1000.0;
    ui->lineEdit_i_a_yc->setText(QString::number(i_a));

    float i_b = ((readData[14]<<8) + readData[15])/1000.0;
    ui->lineEdit_i_b_yc->setText(QString::number(i_b));

    float i_c = ((readData[16]<<8) + readData[17])/1000.0;
    ui->lineEdit_i_c_yc->setText(QString::number(i_c));

    float p_a = ((qint16)((readData[18]<<8) + readData[19]));
    ui->lineEdit_p_a_yc->setText(QString::number(p_a));

    float p_b = ((qint16)((readData[20]<<8) + readData[21]));
    ui->lineEdit_p_b_yc->setText(QString::number(p_b));

    float p_c = ((qint16)((readData[22]<<8) + readData[23]));
    ui->lineEdit_p_c_yc->setText(QString::number(p_c));

    float q_a = ((qint16)((readData[24]<<8) + readData[25]));
    ui->lineEdit_q_a_yc->setText(QString::number(q_a));
    float q_b = ((qint16)((readData[26]<<8) + readData[27]));
    ui->lineEdit_q_b_yc->setText(QString::number(q_b));
    float q_c = ((qint16)((readData[28]<<8) + readData[29]));
    ui->lineEdit_q_c_yc->setText(QString::number(q_c));

    float pf = ((readData[30]<<8) + readData[31])/100.0;
    ui->lineEdit_pf_yc->setText(QString::number(pf));

    float freq = ((readData[32]<<8) + readData[33])/100.0;
    ui->lineEdit_freq_yc->setText(QString::number(freq));

    float dev_temp = ((readData[34]<<8) + readData[35])/10.0;
    ui->lineEdit_temp_yc->setText(QString::number(dev_temp));

    float oil_temp = ((readData[36]<<8) + readData[37])/10.0;
    ui->lineEdit_oil_tmp->setText(QString::number(oil_temp));
}
void YZTY::send_heart()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = HEART_CODE;
    sendData[4] = SEND_HEART_LEN >> 8;
    sendData[5] = SEND_HEART_LEN & 0xFF;
    crc = CRC_16(sendData, SEND_HEART_LEN - 2);
    sendData[6] = crc >> 8;
    sendData[7] = crc & 0xFF;
    send_data((const char *)sendData, SEND_HEART_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送心跳命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_HEART_LEN, sendData, 0);
}

void YZTY::on_checkBox_16_ts_clicked()
{
    if(ui->checkBox_16_ts->isChecked() == true)
        flag_16_ts = true;
    else
        flag_16_ts = false;
}
void YZTY::send_get_device_time()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = GET_T_CODE;
    sendData[4] = SEND_GET_T_LEN >> 8;
    sendData[5] = SEND_GET_T_LEN & 0xFF;
    crc = CRC_16(sendData, SEND_GET_T_LEN - 2);
    sendData[6] = crc >> 8;
    sendData[7] = crc & 0xFF;
    send_data((const char *)sendData, SEND_GET_T_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送获取时间命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_GET_T_LEN, sendData, 0);
}
void YZTY::on_get_device_time_clicked()
{
    sendCode = GET_T_CODE;
}
void YZTY::get_t_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("获取时间命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    char chBuf[20];
    QString s1;
    sprintf(chBuf, "%d-%02d-%02d %02d:%02d:%02d ", readData[7]+2000,
            readData[9], readData[11], readData[13], readData[15], readData[17]);
    s1 = QString(QLatin1String(chBuf));
    ui->device_time->setText(s1);
}
void YZTY::send_set_device_time()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = SET_T_CODE;
    sendData[4] = SEND_SET_T_LEN >> 8;
    sendData[5] = SEND_SET_T_LEN & 0xFF;

    sendData[6] = (ui->master_time->date().year() - 2000) >> 8;
    sendData[7] = (ui->master_time->date().year() - 2000) & 0xFF;
    sendData[8] = (ui->master_time->date().month()) >> 8;
    sendData[9] = (ui->master_time->date().month()) & 0xFF;
    sendData[10] = (ui->master_time->date().day()) >> 8;
    sendData[11] = (ui->master_time->date().day()) & 0xFF;
    sendData[12] = (ui->master_time->time().hour()) >> 8;
    sendData[13] = (ui->master_time->time().hour()) & 0xFF;
    sendData[14] = (ui->master_time->time().minute()) >> 8;
    sendData[15] = (ui->master_time->time().minute()) & 0xFF;
    sendData[16] = (ui->master_time->time().second()) >> 8;
    sendData[17] = (ui->master_time->time().second()) & 0xFF;


    crc = CRC_16(sendData, SEND_SET_T_LEN - 2);
    sendData[SEND_SET_T_LEN - 2] = crc >> 8;
    sendData[SEND_SET_T_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_SET_T_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送配置时间命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_SET_T_LEN, sendData, 0);
}
void YZTY::on_set_device_time_clicked()
{
    sendCode = SET_T_CODE;
}
void YZTY::set_t_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("配置时间命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    char chBuf[20];
    QString s1;
    sprintf(chBuf, "%d-%02d-%02d %02d:%02d:%02d ", readData[7]+2000,
            readData[9], readData[11], readData[13], readData[15], readData[17]);
    s1 = QString(QLatin1String(chBuf));
    ui->device_time->setText(s1);
}
void YZTY::on_get_master_time_clicked()
{
    if(ui->get_master_time->isChecked() == true)
        flag_auto_get_master_t = true;
    else
        flag_auto_get_master_t = false;
}
void YZTY::on_auto_send_heart_clicked()
{
    if(ui->auto_send_heart->isChecked() == true)
        flag_auto_send_heart = true;
    else
        flag_auto_send_heart = false;
}
void YZTY::on_hand_send_heart_clicked()
{
    sendCode = HEART_CODE;
}
void YZTY::send_ty_mode_switch()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = MODE_CODE;
    sendData[4] = SEND_MODE_LEN >> 8;
    sendData[5] = SEND_MODE_LEN & 0xFF;
    if(ui->ty_mode_select->currentIndex() == 0)
        sendData[6] = 0;
    else
        sendData[6] = 1;

    crc = CRC_16(sendData, SEND_MODE_LEN - 2);
    sendData[SEND_MODE_LEN - 2] = crc >> 8;
    sendData[SEND_MODE_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_MODE_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送模式切换命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_MODE_LEN, sendData, 0);
}
void YZTY::on_ty_mode_switch_clicked()
{
    sendCode = MODE_CODE;
}
void YZTY::mode_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("模式切换命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
 //   QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("模式切换成功！"));
}
void YZTY::send_yk_switch()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = YK_CODE;
    sendData[4] = SEND_YK_LEN >> 8;
    sendData[5] = SEND_YK_LEN & 0xFF;
    if(ui->yk_mode_select->currentIndex() == 0)
        sendData[6] = 1;    //升压
    else if(ui->yk_mode_select->currentIndex() == 1)
        sendData[6] = 2;    //降压
    else if(ui->yk_mode_select->currentIndex() == 2)
        sendData[6] = 3;    //调容
    crc = CRC_16(sendData, SEND_YK_LEN - 2);
    sendData[SEND_YK_LEN - 2] = crc >> 8;
    sendData[SEND_YK_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_YK_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送遥控命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_YK_LEN, sendData, 0);
}
void YZTY::on_yk_switch_clicked()
{
    sendCode = YK_CODE;
}
void YZTY::send_yk_enter_switch()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = YK_EN_CODE;
    sendData[4] = SEND_YK_EN_LEN >> 8;
    sendData[5] = SEND_YK_EN_LEN & 0xFF;
    if(ui->yk_enter_mode_select->currentIndex() == 0)
        sendData[6] = 0xFF;
    else
        sendData[6] = 0;
    crc = CRC_16(sendData, SEND_YK_EN_LEN - 2);
    sendData[SEND_YK_EN_LEN - 2] = crc >> 8;
    sendData[SEND_YK_EN_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_YK_EN_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送遥控确认命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_YK_EN_LEN, sendData, 0);
}
void YZTY::on_yk_enter_switch_clicked()
{
    sendCode = YK_EN_CODE;
}

void YZTY::yk_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("遥控命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
//    QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("遥控命令发送成功！"));
}
void YZTY::yk_enter_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("遥控确认命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
//    QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("遥控确认命令发送成功！"));
}
void YZTY::on_get_dev_cfg_clicked()
{
    sendCode = GET_CFG_CODE;
}
void YZTY::send_get_device_cfg()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = GET_CFG_CODE;
    sendData[4] = SEND_GET_CFG_LEN >> 8;
    sendData[5] = SEND_GET_CFG_LEN & 0xFF;
    crc = CRC_16(sendData, SEND_GET_CFG_LEN - 2);
    sendData[SEND_GET_CFG_LEN - 2] = crc >> 8;
    sendData[SEND_GET_CFG_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_GET_CFG_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送获取参数命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_GET_CFG_LEN, sendData, 0);
}
void YZTY::get_device_cfg_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("获取参数命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    int ty_high      = (readData[6 ] << 8) + readData[7 ];
    int ty_low       = (readData[8 ] << 8) + readData[9 ];
    int ty_space     = (readData[10] << 8) + readData[11];
    int ty_delay     = (readData[12] << 8) + readData[13];
    int ct_ratio     = (readData[14] << 8) + readData[15];
    int bh_over_cur  = (readData[16] << 8) + readData[17];
    int bh_ty_high   = (readData[18] << 8) + readData[19];
    int bh_ty_low    = (readData[20] << 8) + readData[21];
    int bh_high_vol  = (readData[22] << 8) + readData[23];
    int bh_low_vol   = (readData[24] << 8) + readData[25];
    int dev_tr_th    = (readData[26] << 8) + readData[27];
    int dev_tr_delay = (readData[28] << 8) + readData[29];
    int bh_oil_tmp   = (readData[30] << 8) + readData[31];
    int tran_content = (readData[32] << 8) + readData[33];
    int dev_code     = (readData[34] << 8) + readData[35];
    int switch_total = (readData[36] << 8) + readData[37];
    int tran_range_h = (readData[38] << 8) + readData[39];
    int tran_range_l = (readData[40] << 8) + readData[41];
    int switch_type  = (readData[42] << 8) + readData[43];
    int dev_h_v      = (readData[44] << 8) + readData[45];
    int dev_s_v      = (readData[46] << 8) + readData[47];

    ui->dev_ty_high->setText(QString::number(ty_high));
    ui->dev_ty_low->setText(QString::number(ty_low));
    ui->dev_ty_space->setText(QString::number(ty_space));
    ui->dev_ty_delay->setText(QString::number(ty_delay));
    ui->bh_over_cur->setText(QString::number(bh_over_cur));
    ui->dev_ct_ratio->setText(QString::number(ct_ratio));
    ui->bh_ty_high->setText(QString::number(bh_ty_high));
    ui->bh_ty_low->setText(QString::number(bh_ty_low));
    ui->bh_high_vol->setText(QString::number(bh_high_vol));
    ui->bh_low_vol->setText(QString::number(bh_low_vol));


    ui->tran_content->setText(QString::number(tran_content));
    ui->tran_switch_total->setText(QString::number(switch_total));
    ui->dev_tr_th->setText(QString::number(dev_tr_th));
    ui->tran_high_per->setText(QString::number(tran_range_h/100));
    ui->tran_low_per->setText(QString::number(tran_range_l/100));
    ui->dev_code->setText(QString::number(dev_code));
    ui->dev_tr_delay->setText(QString::number(dev_tr_delay));
    ui->bh_oil_tmp->setText(QString::number(bh_oil_tmp));

    QString s1;
    s1 = QString::number((dev_h_v/1000)%10) + "." + QString::number((dev_h_v/100)%10) + "." +
                 QString::number((dev_h_v/10)%10) + "." + QString::number(dev_h_v%10);
    ui->dev_h_version->setText(s1);
    QString s2;
    s2 = QString::number((dev_s_v/1000)%10) + "." + QString::number((dev_s_v/100)%10) + "." +
                 QString::number((dev_s_v/10)%10) + "." + QString::number(dev_s_v%10);
    ui->dev_s_version->setText(s2);
    if(switch_type == 0x01)
        ui->switch_type->setText(QString::fromLocal8Bit("直流电机调压开关5档130转"));
    else if(switch_type == 0x02)
        ui->switch_type->setText(QString::fromLocal8Bit("直流电机调压开关5档220转"));
    else if(switch_type == 0x11)
        ui->switch_type->setText(QString::fromLocal8Bit("步进电机调压开关5档"));
    else if(switch_type == 0x12)
        ui->switch_type->setText(QString::fromLocal8Bit("步进电机调压开关9档"));
    else if(switch_type == 0x13)
        ui->switch_type->setText(QString::fromLocal8Bit("步进电机调压开关17档"));
    else if(switch_type == 0x21)
        ui->switch_type->setText(QString::fromLocal8Bit("永磁调容调压开关3档"));
    else if(switch_type == 0x22)
        ui->switch_type->setText(QString::fromLocal8Bit("永磁调压开关5档"));
    else if(switch_type == 0x23)
        ui->switch_type->setText(QString::fromLocal8Bit("永磁调压开关7档"));
    else
        ui->switch_type->setText(QString::number(switch_type));
}
void YZTY::send_set_device_cfg()
{
    unsigned short crc = 0;
    sendData[0 ] = HEAD_H;
    sendData[1 ] = HEAD_L;
    sendData[2 ] = DEVICE_ID;
    sendData[3 ] = SET_CFG_CODE;
    sendData[4 ] = SEND_SET_CFG_LEN >> 8;
    sendData[5 ] = SEND_SET_CFG_LEN & 0xFF;

    sendData[6 ] = ui->dev_ty_high->text().toInt() >> 8;
    sendData[7 ] = ui->dev_ty_high->text().toInt() &  0xFF;
    sendData[8 ] = ui->dev_ty_low->text().toInt() >> 8;
    sendData[9 ] = ui->dev_ty_low->text().toInt() &  0xFF;
    sendData[10] = ui->dev_ty_space->text().toInt() >> 8;
    sendData[11] = ui->dev_ty_space->text().toInt() &  0xFF;
    sendData[12] = ui->dev_ty_delay->text().toInt() >> 8;
    sendData[13] = ui->dev_ty_delay->text().toInt() &  0xFF;
    sendData[14] = ui->dev_ct_ratio->text().toInt() >> 8;
    sendData[15] = ui->dev_ct_ratio->text().toInt() &  0xFF;
    sendData[16] = ui->bh_over_cur->text().toInt() >> 8;
    sendData[17] = ui->bh_over_cur->text().toInt() &  0xFF;
    sendData[18] = ui->bh_ty_high->text().toInt() >> 8;
    sendData[19] = ui->bh_ty_high->text().toInt() &  0xFF;
    sendData[20] = ui->bh_ty_low->text().toInt() >> 8;
    sendData[21] = ui->bh_ty_low->text().toInt() &  0xFF;
    sendData[22] = ui->bh_high_vol->text().toInt() >> 8;
    sendData[23] = ui->bh_high_vol->text().toInt() &  0xFF;
    sendData[24] = ui->bh_low_vol->text().toInt() >> 8;
    sendData[25] = ui->bh_low_vol->text().toInt() &  0xFF;

    sendData[26] = ui->dev_tr_th->text().toInt() >> 8;
    sendData[27] = ui->dev_tr_th->text().toInt() &  0xFF;
    sendData[28] = ui->dev_tr_delay->text().toInt() >> 8;
    sendData[29] = ui->dev_tr_delay->text().toInt() &  0xFF;
    sendData[30] = ui->bh_oil_tmp->text().toInt() >> 8;
    sendData[31] = ui->bh_oil_tmp->text().toInt() &  0xFF;


    sendData[32] = ui->tran_content->text().toInt() >> 8;
    sendData[33] = ui->tran_content->text().toInt() &  0xFF;

    sendData[34] = ui->dev_code->text().toInt() >> 8;
    sendData[35] = ui->dev_code->text().toInt() & 0xFF;

    crc = CRC_16(sendData, SEND_SET_CFG_LEN - 2);
    sendData[SEND_SET_CFG_LEN - 2] = crc >> 8;
    sendData[SEND_SET_CFG_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_SET_CFG_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送配置参数命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_SET_CFG_LEN, sendData, 0);
}
void YZTY::set_device_cfg_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("配置参数命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
//    QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("配置参数成功！"));
}
void YZTY::on_set_dev_cfg_clicked()
{
    sendCode = SET_CFG_CODE;
}
void YZTY::send_get_ty_rec()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = GET_TY_REC_CODE;
    sendData[4] = SEND_GET_TY_REC_LEN >> 8;
    sendData[5] = SEND_GET_TY_REC_LEN & 0xFF;
    int recBegin = ui->ty_rec_begin->text().toInt();
    int recEnd = ui->ty_rec_end->text().toInt();
    sendData[6] =  recBegin  >> 8;
    sendData[7] =  recBegin & 0xff;
    sendData[8] =  recEnd  >> 8;
    sendData[9] =  recEnd & 0xff;
    if(recBegin <= 0)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("开始必须大于0！"));
        return;
    }
    if(recEnd < recBegin)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("结束必须大于等于开始！"));
        return;
    }
    if(recEnd > 150)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("动作记录结束必须小于151！"));
        return;
    }
    crc = CRC_16(sendData, SEND_GET_TY_REC_LEN - 2);
    sendData[SEND_GET_TY_REC_LEN - 2] = crc >> 8;
    sendData[SEND_GET_TY_REC_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_GET_TY_REC_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送获取动作记录命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_GET_TY_REC_LEN, sendData, 0);
}
void YZTY::get_ty_rec_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    int end = (readData[6] << 8) + readData[7];
    int pos = (readData[8] << 8) + readData[9];
    QString s = QString::fromLocal8Bit("获取第") + QString::number(pos) + QString::fromLocal8Bit("条动作记录：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    line++;
    ui->ty_display->setRowCount(line);
    char temp[20];
    sprintf(temp, "%d/%02d/%02d %02d:%02d:%02d", readData[10]+2000, readData[11], readData[12], readData[13], readData[14], readData[15]);
    QString date_buf = temp;
    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[16]<<8) + readData[17])/10.0);
    QString ua_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[18]<<8) + readData[19])/10.0);
    QString ub_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[20]<<8) + readData[21])/10.0);
    QString uc_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[22]<<8) + readData[23])/1000.0);
    QString ia_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[24]<<8) + readData[25])/1000.0);
    QString ib_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[26]<<8) + readData[27])/1000.0);
    QString ic_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.2f", readData[28]/100.0);
    QString pf_buf = temp;


    QString gear_buf = QString::number(readData[29] >> 1);

    QString cap_buf;
    if((readData[29] & 0x01) == 1)
        cap_buf = QString::fromLocal8Bit("小");
    else
        cap_buf = QString::fromLocal8Bit("大");

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[30]<<8) + readData[31])/10.0);
    QString ty_hou_ua_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[32]<<8) + readData[33])/10.0);
    QString ty_hou_ub_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[34]<<8) + readData[35])/10.0);
    QString ty_hou_uc_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[36]<<8) + readData[37])/1000.0);
    QString ty_hou_ia_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[38]<<8) + readData[39])/1000.0);
    QString ty_hou_ib_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[40]<<8) + readData[41])/1000.0);
    QString ty_hou_ic_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.2f", readData[42]/100.0);
    QString ty_hou_pf_buf = temp;


    QString ty_hou_gear_buf = QString::number(readData[43] >> 1);

    QString ty_hou_cap_buf;
    if((readData[43] & 0x01) == 1)
        ty_hou_cap_buf = QString::fromLocal8Bit("小");
    else
        ty_hou_cap_buf = QString::fromLocal8Bit("大");


    QString last_motion_buf;
    if(readData[44] >> 1 == 1)
            last_motion_buf = QString::fromLocal8Bit("升档");
    else if(readData[44] >> 1 == 2)
            last_motion_buf = QString::fromLocal8Bit("降档");
    else if(readData[44] >> 1 == 3)
            last_motion_buf = QString::fromLocal8Bit("调容");
    QString ty_hou_mode_buf;
    if((readData[44] & 0x01) == 0)
        ty_hou_mode_buf = QString::fromLocal8Bit("自动");
    else
        ty_hou_mode_buf = QString::fromLocal8Bit("手动");

    ui->ty_display->setItem(line-1, 0, new QTableWidgetItem(date_buf));
    ui->ty_display->setItem(line-1, 1, new QTableWidgetItem(ua_buf));
    ui->ty_display->setItem(line-1, 2, new QTableWidgetItem(ub_buf));
    ui->ty_display->setItem(line-1, 3, new QTableWidgetItem(uc_buf));
    ui->ty_display->setItem(line-1, 4, new QTableWidgetItem(ia_buf));
    ui->ty_display->setItem(line-1, 5, new QTableWidgetItem(ib_buf));
    ui->ty_display->setItem(line-1, 6, new QTableWidgetItem(ic_buf));
    ui->ty_display->setItem(line-1, 7, new QTableWidgetItem(pf_buf));

    ui->ty_display->setItem(line-1, 8, new QTableWidgetItem(gear_buf));
    ui->ty_display->setItem(line-1, 9, new QTableWidgetItem(cap_buf));
    ui->ty_display->setItem(line-1, 10, new QTableWidgetItem(ty_hou_ua_buf));
    ui->ty_display->setItem(line-1, 11, new QTableWidgetItem(ty_hou_ub_buf));
    ui->ty_display->setItem(line-1, 12, new QTableWidgetItem(ty_hou_uc_buf));
    ui->ty_display->setItem(line-1, 13, new QTableWidgetItem(ty_hou_ia_buf));
    ui->ty_display->setItem(line-1, 14, new QTableWidgetItem(ty_hou_ib_buf));
    ui->ty_display->setItem(line-1, 15, new QTableWidgetItem(ty_hou_ic_buf));
    ui->ty_display->setItem(line-1, 16, new QTableWidgetItem(ty_hou_pf_buf));
    ui->ty_display->setItem(line-1, 17, new QTableWidgetItem(ty_hou_gear_buf));
    ui->ty_display->setItem(line-1, 18, new QTableWidgetItem(ty_hou_cap_buf));
    ui->ty_display->setItem(line-1, 19, new QTableWidgetItem(last_motion_buf));
    ui->ty_display->setItem(line-1, 20, new QTableWidgetItem(ty_hou_mode_buf));
    if(pos == end)
        end_rec_handle();

}
void YZTY::on_get_ty_record_clicked()
{
    sendCode = GET_TY_REC_CODE;
    ui->stackedWidget_rec->setCurrentIndex(0);
}
void YZTY::send_get_ala_rec()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = GET_ALA_REC_CODE;
    sendData[4] = SEND_GET_ALA_REC_LEN >> 8;
    sendData[5] = SEND_GET_ALA_REC_LEN & 0xFF;
    int recBegin = ui->ala_rec_begin->text().toInt();
    int recEnd = ui->ala_rec_end->text().toInt();
    sendData[6] =  recBegin  >> 8;
    sendData[7] =  recBegin & 0xff;
    sendData[8] =  recEnd  >> 8;
    sendData[9] =  recEnd & 0xff;
    if(recBegin <= 0)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("开始必须大于0！"));
        return;
    }
    if(recEnd < recBegin)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("结束必须大于等于开始！"));
        return;
    }
    if(recEnd > 50)
    {
        QMessageBox::about(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("告警记录结束必须小于51！"));
        return;
    }
    crc = CRC_16(sendData, SEND_GET_ALA_REC_LEN - 2);
    sendData[SEND_GET_ALA_REC_LEN - 2] = crc >> 8;
    sendData[SEND_GET_ALA_REC_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_GET_ALA_REC_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送获取告警记录命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_GET_ALA_REC_LEN, sendData, 0);
}
void YZTY::get_ala_rec_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    int end = (readData[6] << 8) + readData[7];
    int pos = (readData[8] << 8) + readData[9];
    QString s = QString::fromLocal8Bit("获取第") + QString::number(pos) + QString::fromLocal8Bit("条告警记录：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    line++;
    ui->ala_display->setRowCount(line);
    char temp[20];
    sprintf(temp, "%d/%02d/%02d %02d:%02d:%02d", readData[10]+2000, readData[11], readData[12], readData[13], readData[14], readData[15]);
    QString date_buf = temp;
    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[16]<<8) + readData[17])/10.0);
    QString ua_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[18]<<8) + readData[19])/10.0);
    QString ub_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.1f", ((readData[20]<<8) + readData[21])/10.0);
    QString uc_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[22]<<8) + readData[23])/1000.0);
    QString ia_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[24]<<8) + readData[25])/1000.0);
    QString ib_buf = temp;

    memset(temp, 0, 20);
    sprintf(temp, "%.3f", ((readData[26]<<8) + readData[27])/1000.0);
    QString ic_buf = temp;
    QString gear_buf = QString::number(readData[28] >> 1);

    QString cap_buf;
    if((readData[28] & 0x01) == 1)
        cap_buf = QString::fromLocal8Bit("小");
    else
        cap_buf = QString::fromLocal8Bit("大");

    int device_ago_state = (readData[30]<<8) +  readData[31];
    int device_state = (readData[32]<<8) +  readData[33];
    QString device_ago_state_buf;
    if(device_ago_state == 0)
        device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("正常");
    else
    {
        if((device_ago_state & 0x01) == 0x01)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("初始化失败 | ");
        if((device_ago_state & 0x02) == 0x02)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("采样故障 | ");
        if((device_ago_state & 0x04) == 0x04)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("存储故障 | ");
        if((device_ago_state & 0x08) == 0x08)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("档位故障 | ");
        if((device_ago_state & 0x10) == 0x10)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("电机故障 | ");
        if((device_ago_state & 0x20) == 0x20)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("失电告警 | ");
        if((device_ago_state & 0x40) == 0x40)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("过流告警 | ");
        if((device_ago_state & 0x80) == 0x80)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("低压告警 | ");
        if((device_ago_state & 0x100) == 0x100)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("高压告警 | ");
        if((device_ago_state & 0x200) == 0x200)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("开关闭锁 | ");
        if((device_ago_state & 0x400) == 0x400)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("油位告警 | ");
        if((device_ago_state & 0x800) == 0x800)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("油温告警 | ");
        if((device_ago_state & 0x1000) == 0x1000)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("调档失败 | ");
        if((device_ago_state & 0x2000) == 0x2000)
            device_ago_state_buf = device_ago_state_buf + QString::fromLocal8Bit("调容失败 | ");
    }

    QString device_state_buf;
    if(device_state == 0)
    {
        device_state_buf = device_state_buf + QString::fromLocal8Bit("正常");
    }
    else
    {
        if((device_state & 0x01) == 0x01)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("初始化失败 | ");
        if((device_state & 0x02) == 0x02)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("采样故障 | ");
        if((device_state & 0x04) == 0x04)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("存储故障 | ");
        if((device_state & 0x08) == 0x08)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("档位故障 | ");
        if((device_state & 0x10) == 0x10)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("电机故障 | ");
        if((device_state & 0x20) == 0x20)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("失电告警 | ");
        if((device_state & 0x40) == 0x40)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("过流告警 | ");
        if((device_state & 0x80) == 0x80)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("低压告警 | ");
        if((device_state & 0x100) == 0x100)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("高压告警 | ");
        if((device_state & 0x200) == 0x200)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("开关闭锁 | ");
        if((device_state & 0x400) == 0x400)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("油位告警 | ");
        if((device_state & 0x800) == 0x800)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("油温告警 | ");
        if((device_state & 0x1000) == 0x1000)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("调档失败 | ");
        if((device_state & 0x2000) == 0x2000)
            device_state_buf = device_state_buf + QString::fromLocal8Bit("调容失败 | ");
    }

    ui->ala_display->setItem(line-1, 0, new QTableWidgetItem(date_buf));
    ui->ala_display->setItem(line-1, 1, new QTableWidgetItem(ua_buf));
    ui->ala_display->setItem(line-1, 2, new QTableWidgetItem(ub_buf));
    ui->ala_display->setItem(line-1, 3, new QTableWidgetItem(uc_buf));
    ui->ala_display->setItem(line-1, 4, new QTableWidgetItem(ia_buf));
    ui->ala_display->setItem(line-1, 5, new QTableWidgetItem(ib_buf));
    ui->ala_display->setItem(line-1, 6, new QTableWidgetItem(ic_buf));

    ui->ala_display->setItem(line-1, 7, new QTableWidgetItem(gear_buf));
    ui->ala_display->setItem(line-1, 8, new QTableWidgetItem(cap_buf));
    ui->ala_display->setItem(line-1, 9, new QTableWidgetItem(QString::number(readData[29])));;

    ui->ala_display->setItem(line-1, 10, new QTableWidgetItem(device_ago_state_buf));
    ui->ala_display->setItem(line-1, 11, new QTableWidgetItem(device_state_buf));

    if(pos == end)
        end_rec_handle();
}
void YZTY::on_get_alarm_record_clicked()
{
    sendCode = GET_ALA_REC_CODE;
    ui->stackedWidget_rec->setCurrentIndex(1);
}
void YZTY::end_rec_handle()
{
    line = 0;
    QMessageBox::information(NULL,QString::fromLocal8Bit("通知"),QString::fromLocal8Bit("获取记录完成！"));
}
void YZTY::on_save_rec_clicked()
{
    if(ui->save_rec_select->currentIndex() == 0)
        table_to_excel_by_txt(ui->ty_display);
    else if(ui->save_rec_select->currentIndex() == 1)
        table_to_excel_by_txt(ui->ala_display);
}
void YZTY::adjust_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("校表命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
}
void YZTY::send_adjust()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = ADJUST_CODE;
    sendData[4] = SEND_ADJUST_LEN >> 8;
    sendData[5] = SEND_ADJUST_LEN & 0xFF;
    int adjust_u  = ui->adjust_v->text().toFloat()*10;
    int adjust_i  = ui->adjust_i->text().toFloat()*1000;
    sendData[6] = adjust_u >> 24;
    sendData[7] = (adjust_u >> 16) & 0xFF;
    sendData[8] = (adjust_u >> 8) & 0xFF;
    sendData[9] = adjust_u & 0xFF;
    sendData[10] = adjust_i >> 24;
    sendData[11] = (adjust_i >> 16) & 0xFF;
    sendData[12] = (adjust_i >> 8) & 0xFF;
    sendData[13] = adjust_i & 0xFF;
    crc = CRC_16(sendData, SEND_ADJUST_LEN - 2);
    sendData[SEND_ADJUST_LEN - 2] = crc >> 8;
    sendData[SEND_ADJUST_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_ADJUST_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送校表命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_ADJUST_LEN, sendData, 0);
}
void YZTY::on_adjust_clicked()
{
    sendCode = ADJUST_CODE;
}
void YZTY::reboot_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("重启命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    if(reboot_reason == 1)
        flag_program = true;
    else
    {
        QString s1 = QString::fromLocal8Bit("设备正在重启，请等待") + QString::number(rebootTime) + QString::fromLocal8Bit("秒！");
        QMessageBox::information(NULL,QString::fromLocal8Bit("通知"), s1);
    }
}
void YZTY::on_select_ty_ala_clicked()
{
    if(ui->stackedWidget_rec->currentIndex() == 0)
        ui->stackedWidget_rec->setCurrentIndex(1);
    else if(ui->stackedWidget_rec->currentIndex() == 1)
        ui->stackedWidget_rec->setCurrentIndex(0);
}
void YZTY::send_reboot()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = REBOOT_CODE;
    sendData[4] = SEND_REBOOT_LEN >> 8;
    sendData[5] = SEND_REBOOT_LEN & 0xFF;
    sendData[6] = 0xFF;
    crc = CRC_16(sendData, SEND_REBOOT_LEN - 2);
    sendData[SEND_REBOOT_LEN - 2] = crc >> 8;
    sendData[SEND_REBOOT_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_REBOOT_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送重启命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_REBOOT_LEN, sendData, 0);
}
void YZTY::on_reboot_clicked()
{
    sendCode = REBOOT_CODE;
    reboot_reason = 0;
}
void YZTY::clr_rec_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("清除记录命令回复：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 0);
    QMessageBox::information(NULL,QString::fromLocal8Bit("通知"), QString::fromLocal8Bit("清除记录成功！"));
}
void YZTY::send_clr_rec()
{
    unsigned short crc = 0;
    sendData[0] = HEAD_H;
    sendData[1] = HEAD_L;
    sendData[2] = DEVICE_ID;
    sendData[3] = CLR_REC_CODE;
    sendData[4] = SEND_CLR_REC_LEN >> 8;
    sendData[5] = SEND_CLR_REC_LEN & 0xFF;
    if(ui->clear_rec_select->currentIndex() == 0)
        sendData[6] = 0;
    else if(ui->clear_rec_select->currentIndex() == 1)
        sendData[6] = 1;
    else if(ui->clear_rec_select->currentIndex() == 2)
        sendData[6] = 2;
    crc = CRC_16(sendData, SEND_CLR_REC_LEN - 2);
    sendData[SEND_CLR_REC_LEN - 2] = crc >> 8;
    sendData[SEND_CLR_REC_LEN - 1] = crc & 0xFF;
    send_data((const char *)sendData, SEND_CLR_REC_LEN);
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("主机发送清除记录命令：");
    ui->textEdit_print_ts->append(s);
    print(SEND_CLR_REC_LEN, sendData, 0);
}
void YZTY::on_clear_rec_clicked()
{
    sendCode = CLR_REC_CODE;
}
void YZTY::on_select_cfg_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("调用配置文件"),
            QString(), ("*.ini"));
    QSettings cfgini(filepath, QSettings::IniFormat);
    if(cfgini.contains("device_code/code"))
    {
        ui->dev_ty_high->setText(cfgini.value("device/ty_high").toString());
        ui->dev_ty_low->setText(cfgini.value("device/ty_low").toString());
        ui->dev_ty_space->setText(cfgini.value("device/ty_space").toString());
        ui->dev_ty_delay->setText(cfgini.value("device/ty_delay").toString());
        ui->dev_ct_ratio->setText(cfgini.value("device/ct_ratio").toString());
        ui->dev_tr_delay->setText(cfgini.value("device/tr_delay").toString());
        ui->dev_tr_th->setText(cfgini.value("device/tr_th").toString());
        ui->dev_code->setText(cfgini.value("device_code/code").toString());

        ui->bh_over_cur->setText(cfgini.value("bh/over_cur").toString());
        ui->bh_high_vol->setText(cfgini.value("bh/high_vol").toString());
        ui->bh_low_vol->setText(cfgini.value("bh/low_vol").toString());
        ui->bh_ty_high->setText(cfgini.value("bh/bs_high").toString());
        ui->bh_ty_low->setText(cfgini.value("bh/bs_low").toString());
        ui->bh_oil_tmp->setText(cfgini.value("bh/oil_tmp").toString());


        ui->tran_content->setText(cfgini.value("tran/tran_con").toString());
        ui->tran_switch_total->setText(cfgini.value("tran/tap_num").toString());

        ui->tran_high_per->setText(cfgini.value("tran/tap_h").toString());
        ui->tran_low_per->setText(cfgini.value("tran/tap_l").toString());
    }
}
void YZTY::error_handle(int len)
{
    ui->textEdit_print_ts->setTextColor(Qt::blue);
    QString s = QString::fromLocal8Bit("设备回复错误命令：");
    ui->textEdit_print_ts->append(s);
    print(len, readData, 1);
    int error_code = (readData[6] << 8) + readData[7];
    switch(error_code)
    {
    case 401:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("校验和错误！"));
        break;
    case 402:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("未收到遥控命令！"));
        break;
    case 403:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("参数越界！"));
        break;
    case 404:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("参数保存失败！"));
        break;
    case 405:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("开关闭锁，禁止遥控！"));
        break;
    case 406:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("档位已最小，禁止降档！"));
        break;
    case 407:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("档位已最大，禁止升档！"));
        break;
    case 408:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("模式错误！"));
    case 409:
        QMessageBox::warning(NULL,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("校表失败！"));
        break;
    default:
        ;
        break;
    }
}
void YZTY::rev_data_handle()
{
    unsigned short crc = 0;
    crc = (readData[revLength - 2] << 8) +  readData[revLength - 1];
    if(crc == CRC_16(readData, revLength - 2))
    {
        switch(readData[3])
        {
        case HEART_CODE:
            heart_handle(revLength);
            break;
        case YC_CODE:
            yc_handle(revLength);
            break;
        case MODE_CODE:
            mode_handle(revLength);
            break;
        case GET_T_CODE:
            get_t_handle(revLength);
            break;
        case SET_T_CODE:
            set_t_handle(revLength);
            break;
        case YK_CODE:
            yk_handle(revLength);
            break;
        case YK_EN_CODE:
            yk_enter_handle(revLength);
            break;
        case GET_CFG_CODE:
            get_device_cfg_handle(revLength);
            break;
        case SET_CFG_CODE:
            set_device_cfg_handle(revLength);
            break;
        case GET_TY_REC_CODE:
            get_ty_rec_handle(revLength);
            break;
        case GET_ALA_REC_CODE:
            get_ala_rec_handle(revLength);
            break;
        case CLR_REC_CODE:
            clr_rec_handle(revLength);
            break;
        case ADJUST_CODE:
            adjust_handle(revLength);
            break;
        case REBOOT_CODE:
            reboot_handle(revLength);
            break;
        case ERROR_CODE:
            error_handle(revLength);
            break;
        default:
            ;
            break;
        }
    }
    readCount = 0;
    revLength = 0;
}
void YZTY::mainTimer_update()
{
    //ui->textEdit_print_ts->append("mainTimer_update");
    int sendSelect = sendCode; //取出sendCode的值，mainTimer事件的优先级低，防止mainTimer_update函数执行时间长时，sendCode的值被其他事件修改
    if(flag_auto_get_master_t == true)
        ui->master_time->setDateTime(QDateTime::currentDateTime());
    if(isConnetOK == false)
    {
        if(sendSelect != 0)
        {
            QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("串口未打开！"));
            sendCode = 0;
        }
        return;
    }
    if(timeoutHand > 0)
    {
        timeoutHand --;
        return;
    }
    switch(sendSelect)
    {
    case HEART_CODE:
        send_heart();
        break;
    case YC_CODE:
        send_yc();
        break;
    case GET_T_CODE:
        send_get_device_time();
        break;
    case SET_T_CODE:
        send_set_device_time();
        break;
    case MODE_CODE:
        send_ty_mode_switch();
        break;
    case YK_CODE:
        send_yk_switch();
        break;
    case YK_EN_CODE:
        send_yk_enter_switch();
        break;
    case GET_CFG_CODE:
        send_get_device_cfg();
        break;
    case SET_CFG_CODE:
        send_set_device_cfg();
        break;
    case GET_TY_REC_CODE:
        send_get_ty_rec();
        break;
    case GET_ALA_REC_CODE:
        send_get_ala_rec();
        break;
    case CLR_REC_CODE:
        send_clr_rec();
        break;
    case ADJUST_CODE:
        send_adjust();
        break;
    case REBOOT_CODE:
        send_reboot();
        break;
    default:
        break;
    }
    if(sendSelect != 0)
    {
        sendCode    = 0;
        timeoutHand = 10;
        return;   //发送过命令，后面的自动发命令不执行
    }
    if(timeoutAuto > 0)
    {
        timeoutAuto--;
        return;
    }
    if(sendAutoCode == 0)
        sendAutoCode = 1;
    else if(sendAutoCode == 1)
        sendAutoCode = 0;
    if(sendAutoCode == 0)
    {
        if(flag_auto_send_yc == true)
        {
            send_yc();
            timeoutAuto = 20;
        }
    }
    else if(sendAutoCode == 1)
    {
        if(flag_auto_send_heart == true)
        {
            send_heart();
            timeoutAuto = 20;
        }
    }
}
void YZTY::table_to_excel_by_txt(QTableWidget *table)
{
    QString filepath = QFileDialog::getSaveFileName(this, tr("Save as..."),
            QString(), tr("EXCEL files (*.xls *.xlsx);;HTML-Files (*.txt);;"));

    int row = table->rowCount();
    int col = table->columnCount();
    QList<QString> list;
    //添加列标题
    QString HeaderRow;
    for(int i=0; i<col; i++)
    {
        HeaderRow.append(table->horizontalHeaderItem(i)->text()+"\t");
    }
    list.push_back(HeaderRow);
    for(int i=0; i<row; i++)
    {
        QString rowStr = "";
        for(int j=0; j<col; j++){
            rowStr += table->item(i, j)->text() + "\t";
        }
        list.push_back(rowStr);
    }
    QTextEdit textEdit;
    for(int i=0; i<list.size(); i++)
    {
        textEdit.append(list.at(i));
    }
    QFile file(filepath);
    if(file.open(QFile::WriteOnly | QIODevice::Text))
    {
        QTextStream ts(&file);
     //   ts.setCodec("UTF-8");
        ts<<textEdit.document()->toPlainText();
        file.close();
    }
}
void YZTY::on_program_open_bin_file_clicked()
{
    binFilePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开程序软件"),
            QString(), ("*.bin"));
    ui->program_view_bin_file->setText(binFilePath);
}
void YZTY::on_program_download_clicked()
{
    int choose;
    QString s;
    choose = QMessageBox::question(this, QString::fromLocal8Bit("下载程序"),
                                       QString::fromLocal8Bit("确认下载程序?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if(choose == QMessageBox::No)
        return;
    if (binFilePath.isEmpty())//如果未选择文件便确认，即返回
    {
        QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("未选择文件！"));
        return;
    }
    file.setFileName(binFilePath);
    fileInfo = QFileInfo(file);
    if(file.open(QIODevice::ReadOnly))
    {
        s = "filename:  " + fileInfo.fileName();
        ui->textEdit_print_ts->append(s);
        s = "filesize:  " + QString::number(fileInfo.size());
        qint64 size = fileInfo.size();
        if(size < 1024)
            ymodem_packet_number = 1;
        else if((size % 1024) == 0)
            ymodem_packet_number = fileInfo.size()/1024;
        else
            ymodem_packet_number = (fileInfo.size()/1024) + 1;
        ui->textEdit_print_ts->append(s);
        s = "packet_number:  " + QString::number(ymodem_packet_number);
        ui->textEdit_print_ts->append(s);

    }
    else
        QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("打开文件失败！"));
    if(isConnetOK == true)
    {
        send_reboot();
        reboot_reason = 1;
        if(mainTimer->isActive())
            mainTimer->stop();             //关闭主定时器，开启下载程序定时器
        if(!programTimer->isActive())
            programTimer->start(100);          //开启程序下载定时器
        ui->program_download->setEnabled(false);
        ui->program_open_bin_file->setEnabled(false);
    }
    else
        QMessageBox::warning(NULL,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("串口未打开！"));
}

void YZTY::programTimer_update()
{
   // ui->textEdit_print_ts->append("programTimer_update");
    ymodem_timeout++;
    if(ymodem_packet_number > 0)
        ui->progressBar->setValue(ymodem_packet_counter*100/ymodem_packet_number);
    if(ymodem_timeout == 100)
    {
        ymodem_timeout = 0;
        ymodem_end(2);
    }
}
/*
 * is_end 0 发送开始帧  1 发送结束帧
*/
void YZTY::ymodem_send_SOH(int is_end)
{
    memset(ymodem_send, 0, 133);
    ymodem_send[0] = SOH;
    ymodem_send[1] = 0x00;
    ymodem_send[2] = 0xFF;
    QString s;
    if(is_end == 0)
    {
        int length_filename = 0;
        int length_filesize = 0;
        QByteArray b_filename, b_filesize;
        QString s_filename = fileInfo.fileName();
        b_filename = s_filename.toLatin1();
        QString s_filesize = QString::number(fileInfo.size());
        b_filesize = s_filesize.toLatin1();
        char* filename = b_filename.data();
        char* filesize = b_filesize.data();
        length_filename = fileInfo.fileName().size();
        length_filesize = QString::number(fileInfo.size()).size();
        memcpy(ymodem_send + 3, filename, length_filename);
        memcpy(ymodem_send + 4 + length_filename, filesize, length_filesize);
    }
    uint16_t crc = ymodem_crc16(ymodem_send+3, 128);
    ymodem_send[131] = (crc>>8) & 0xFF;
    ymodem_send[132] =  crc & 0xFF;
    send_data((const char*)ymodem_send, 133);
    s =  QString::fromLocal8Bit("\r\n主机发送SOH: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    ui->textEdit_print_ts->insertPlainText(s);
 //   print(133, ymodem_send, 0);
}

uint16_t YZTY::ymodem_crc16(uint8_t *buff, uint32_t len)
{
    uint16_t crc = 0;
    while(len--)
    {
        crc ^= (uint16_t)(*(buff++)) << 8;
        for(int i = 0; i < 8; i++)
        {
            if(crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc;
}
void YZTY:: ymodem_send_last()
{
    char data = EOT;
    int send_len = 0;
    switch(ymodem_last_send)
    {
    case EOT:
        send_data(&data, 1);
        ui->textEdit_print_ts->append(QString::fromLocal8Bit("主机发送：EOT >>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
        ymodem_packet_counter++;
        return;
    case SOH:
        send_len = 133;
        break;
    case STX:
        send_len = 1029;
        break;
    }
    send_data((const char*)ymodem_send, send_len);
    ui->textEdit_print_ts->insertPlainText(QString::fromLocal8Bit("数据重发: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
 //   print(send_len, ymodem_send, 0);
}
void YZTY:: program_handle(unsigned char cmd)
{
    ymodem_timeout = 0;   //收到正确的数据应答，将超时清零
    char data = EOT;
    if(cmd == CR)
    {
        if(ymodem_last_rec == ACK && ymodem_last_send == SOH)
        {
            ymodem_send_STX();
            ymodem_last_send = STX;
            ymodem_packet_counter++;
        }
        else if(ymodem_last_rec == ACK && ymodem_last_send == EOT)
        {
            ymodem_send_SOH(1);
            ymodem_packet_counter++;
        }
        else if(ymodem_last_rec == CR)
        {
            char data_a = ABORT1;
            send_data(&data_a, 1);
            ui->textEdit_print_ts->append(QString::fromLocal8Bit("主机发送：ABORT1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
            ymodem_end(1);
        }
    }
    else if(cmd == ACK)
    {
        if(ymodem_packet_counter == 0)
            ymodem_packet_counter++;
        else if(ymodem_packet_counter < ymodem_packet_number + 1)
        {
            ymodem_send_STX();
            ymodem_last_send = STX;
            ymodem_packet_counter++;
        }
        else if(ymodem_packet_counter == ymodem_packet_number + 1)
        {
            send_data(&data, 1);
            ui->textEdit_print_ts->append(QString::fromLocal8Bit("主机发送：EOT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
            ymodem_last_send = EOT;
            ymodem_packet_counter++;
        }
        else if(ymodem_packet_counter == ymodem_packet_number + 4)
            ymodem_end(0);
    }
    else if(cmd == NAK)
    {
        ymodem_send_last();
    }
    else if(cmd == CA)
    {
        if(ymodem_last_rec == CA)
            ymodem_end(1);
    }
}
void YZTY::ymodem_end(int stat)
{
    ymodem_last_rec       = 0;
    ymodem_last_send      = 0;
    ymodem_packet_number  = 0;
    ymodem_packet_counter = 0;
    ymodem_timeout        = 0;
    flag_ymodem_start     = false;
    flag_program          = false;
    if(file.isOpen())
        file.close();
    if(!mainTimer->isActive())
        mainTimer->start(100);
    if(programTimer->isActive())
        programTimer->stop();
    ui->program_download->setEnabled(true);
    ui->program_open_bin_file->setEnabled(true);
    if(stat == 0)
    {
        ui->textEdit_print_ts->insertPlainText(QString::fromLocal8Bit("更新成功！"));
        ui->program_stat->setEnabled(true);
        ui->program_stat->setText(QString::fromLocal8Bit("更新成功！"));
        QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("更新成功！"));
    }
    else if(stat == 1)
    {
        ui->textEdit_print_ts->insertPlainText(QString::fromLocal8Bit("设备取消通信，更新失败！"));
        ui->program_stat->setEnabled(true);
        ui->program_stat->setText(QString::fromLocal8Bit("更新失败"));
        QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("更新失败！"));
    }
    else if(stat == 2)
    {
        ui->textEdit_print_ts->insertPlainText(QString::fromLocal8Bit("通信超时，更新失败！"));
        ui->program_stat->setEnabled(true);
        ui->program_stat->setText(QString::fromLocal8Bit("更新失败"));
        QMessageBox::information(NULL,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("更新失败！"));
    }
}
void YZTY::ymodem_send_STX()
{
    int send_len = 0;
    QString s;
    memset(ymodem_send, 0x1A, 1029);
    int len = file.read((char*)(ymodem_send + 3), 1024);
    if(len < 128)
    {
        ymodem_send[0] = SOH;
        send_len = 133;
        s = QString::fromLocal8Bit("\r\n主机发送SOH: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }
    else
    {
        send_len = 1029;
        ymodem_send[0] = STX;
        s = QString::fromLocal8Bit("\r\n主机发送STX: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }
    ymodem_send[1] = ymodem_packet_counter;
    ymodem_send[2] = (ymodem_packet_counter ^ 0xff) & 0xff;

    uint16_t crc = ymodem_crc16(ymodem_send+3, send_len - 5);
    ymodem_send[send_len - 2] = (crc>>8) & 0xFF;
    ymodem_send[send_len - 1] =  crc & 0xFF;

    send_data((const char*)ymodem_send, send_len);
    ui->textEdit_print_ts->insertPlainText(s);
  //  print(send_len, ymodem_send, 0);
}
void YZTY::on_textEdit_print_ts_textChanged()
{
    ui->textEdit_print_ts->moveCursor(QTextCursor::End);
}

