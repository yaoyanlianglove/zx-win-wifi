#ifndef YZTY_H
#define YZTY_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QTableWidget>
#include <QTcpSocket>

#define HEAD_H            0x55
#define HEAD_L            0xaa
#define DEVICE_ID         0x0F

#define HEART_CODE        0x10
#define YC_CODE           0x16
#define GET_T_CODE        0x22
#define SET_T_CODE        0x23
#define MODE_CODE         0x20
#define YK_CODE           0x30
#define YK_EN_CODE        0x31
#define GET_CFG_CODE      0x11
#define SET_CFG_CODE      0x21
#define ERROR_CODE        0x40
#define GET_TY_REC_CODE   0x12
#define GET_ALA_REC_CODE  0x13
#define END_REC_CODE      0x14
#define CLR_REC_CODE      0x15
#define ADJUST_CODE       0x50
#define REBOOT_CODE       0x60

#define SEND_HEART_LEN       8
#define SEND_YC_LEN          8
#define SEND_GET_T_LEN       8
#define SEND_SET_T_LEN       20
#define SEND_MODE_LEN        9
#define SEND_YK_LEN          9
#define SEND_YK_EN_LEN       9
#define SEND_GET_CFG_LEN     8
#define SEND_SET_CFG_LEN     44
#define SEND_GET_TY_REC_LEN  12
#define SEND_GET_ALA_REC_LEN 12
#define SEND_ADJUST_LEN      16
#define SEND_REBOOT_LEN      9
#define SEND_CLR_REC_LEN     9

namespace Ui {
class YZTY;
}

class YZTY : public QMainWindow
{
    Q_OBJECT

public:
    explicit YZTY(QWidget *parent = 0);
    ~YZTY();

private:
    Ui::YZTY *ui;
    QSerialPort *mycom;
    QTimer *mainTimer, *programTimer;  //主定时器  程序下载定时器
    bool isConnetOK;

    QTcpSocket *tcpClient;

    int readCount;
    int revLength;

    unsigned char ymodem_last_send;    //ymodem协议，上一次发送命令
    unsigned char ymodem_last_rec ;    //ymodem协议，上一次接收命令
    unsigned char readData[64];
    unsigned char sendData[64];

    unsigned char ymodem_send[1029];   //ymodem协议，发送数据数组
    bool flag_auto_send_yc;
    bool flag_16_ts;
    bool flag_auto_get_master_t;
    bool flag_auto_send_heart;
    bool flag_program;            //程序下载是否在进行
    bool flag_ymodem_start;       //是否进入Ymodem通信


    int sendCode;              //0不发送命令，其他对应通信协议的功能码，发送命令
    int sendAutoCode;          //发自动命令
    int timeoutHand;           //手动发送命令后的延时
    int timeoutAuto;           //自动发送命令后的延时
    int rebootTime;
    int line;                  //获取历史记录的行数
    int ymodem_timeout;        //程序下载超时
    int reboot_reason;         //0 正常重启， 1 程序下载重启

    unsigned char ymodem_packet_counter; //发送数据包计数
    int ymodem_packet_number;   //程序文件分成的数据包数

    QString binFilePath;       //程序更新文件路径
    QFile file;                //程序文件
    QFileInfo fileInfo;        //程序文件信息


    void ui_init();
    void com_init();
    void net_init();
    void timer_init();

    void rev_data_handle();
    void print(int len, unsigned char* data, int error);
    void send_yc();
    void send_heart();
    void send_get_device_time();
    void send_set_device_time();
    void send_ty_mode_switch();
    void send_yk_switch();
    void send_yk_enter_switch();
    void send_get_device_cfg();
    void send_set_device_cfg();
    void send_get_ty_rec();
    void send_get_ala_rec();
    void send_adjust();
    void send_reboot();
    void send_clr_rec();

    void heart_handle(int len);
    void yc_handle(int len);
    void set_t_handle(int len);
    void get_t_handle(int len);
    void mode_handle(int len);
    void error_handle(int len);
    void yk_handle(int len);
    void yk_enter_handle(int len);
    void get_device_cfg_handle(int len);
    void set_device_cfg_handle(int len);
    void get_ty_rec_handle(int len);
    void get_ala_rec_handle(int len);
    void end_rec_handle();
    void adjust_handle(int len);
    void reboot_handle(int len);
    void clr_rec_handle(int len);


    void table_to_excel_by_txt(QTableWidget *table);
    uint16_t ymodem_crc16(uint8_t *buff, uint32_t len);   //ymodem 校验和
    void ymodem_send_SOH(int is_end);                     //发送ymodem SOH包 is_end 0 开始帧  1 结束帧
    void ymodem_send_STX();                               //发送ymodem 数据包
    void program_handle(unsigned char cmd);               //ymodem通信过程中对收到的数据进行处理
    void ymodem_end(int stat);                            //结束ymodem需要做的处理
    void ymodem_send_last();                              //ymodem数据重发
private slots:
    void read_data();
    void send_data(const char *sendData, int len);
    void mainTimer_update();
    void programTimer_update();                           //程序下载定时函数

    void displayError(QAbstractSocket::SocketError);

    void on_listWidget_nav_itemClicked();
    void on_pushButton_conn_com_clicked();
    void on_pushButton_conn_net_clicked();
    void on_pushButton_empty_ts_clicked();
    void on_checkBox_yc_switch_clicked();
    void on_checkBox_16_ts_clicked();
    void on_get_device_time_clicked();
    void on_set_device_time_clicked();
    void on_get_master_time_clicked();
    void on_ty_mode_switch_clicked();
    void on_auto_send_heart_clicked();
    void on_hand_send_heart_clicked();
    void on_yk_switch_clicked();
    void on_yk_enter_switch_clicked();
    void on_get_yc_switch_clicked();
    void on_get_dev_cfg_clicked();
    void on_set_dev_cfg_clicked();
    void on_get_ty_record_clicked();
    void on_get_alarm_record_clicked();
    void on_save_rec_clicked();
    void on_adjust_clicked();
    void on_reboot_clicked();
    void on_clear_rec_clicked();
    void on_select_cfg_clicked();
    void on_select_ty_ala_clicked();
    void on_program_open_bin_file_clicked();
    void on_program_download_clicked();

    void on_radioButton_net_clicked(bool);
    void on_radioButton_uart_clicked(bool);
    void on_hand_input_code_clicked();

    void on_textEdit_print_ts_textChanged();

};

#endif // YZTY_H
