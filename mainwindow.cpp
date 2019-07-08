#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QtSerialPort>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <QString>
#include <QThread>
#include <QFileDialog>
QT_USE_NAMESPACE;

QSerialPort *serial;
qint8 DLE = 0x10;
qint8 ETX = 0x03;
QByteArray ba_all;
int it_files = 40;
bool iConnect = false;
bool write_to_file = false;
qint16 inc;

struct vec{
    double lat, lon;
    double height;
    double vlon, vlat, vheight;
    long double time;
    float sko;
    qint16 week;
    uint8_t x;
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);                  // Выключаем кнопки запроса на получение
    ui->pushButton_2->setEnabled(false);                // и отключение выдачи данных
    ui->lineEdit->setText("1");                         // Устанавливаем выдачу данныхраз в секунду
    serial = new QSerialPort(this);                     // Создаём экземпляр класса
    serial->setBaudRate(QSerialPort::Baud115200);       // Скорость передачи данных
    //serial->setParity(QSerialPort::OddParity);          // Проверка на нечетность
    serial->setParity(QSerialPort::NoParity);
    serial->setDataBits(QSerialPort::Data8);            // 8 бит данных
    serial->setFlowControl(QSerialPort::NoFlowControl); // Отсутствует управление потоком данных
    serial->setStopBits(QSerialPort::OneStop);          // Один стоп-бит

    connect(serial, SIGNAL(readyRead()), this, SLOT(SerialRecieved())); // По сигналу с readyRead()
                                                                        // передаём данные с COM-порта на вывод
    connect(serial, SIGNAL(readyRead()), this, SLOT(StartWork()));      // ..//.. вызываем функцию StartWork()

    // Вывод всех открытых портов
    QSerialPortInfo serial_info;
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
            ui->comboBox->addItem(port.portName());     // Выводим имена портов в combobox
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();                                    // Закрываем COM-порт
}

void MainWindow::StartWork()
{
    const qint8 connectionOk = 0x54;
    const qint8 vecRec = 0x88;
    qint8 recID = ba_all[1];
    switch (recID) {
    case connectionOk:      // Получен ответ на запрос о соединении
        iConnect = true;
        ui->pushButton->setEnabled(true);
        break;
    case vecRec:            //Получен вектор состояния
        convertInfo(ba_all);
        break;
    }
}

void MainWindow::checkAnswer()
{

}

void MainWindow::getTime(long double time)  // IN - время, мс с начала недели; OUT - день недели и время hh:mm:ss
{
    int new_time = floor(time);
    //QDateTime dow = QDateTime::fromMSecsSinceEpoch(new_time);
    //qDebug() << dow;
    if (new_time < 0)
    {
        QString str_f = "C:\\учеба\\навигационная микросхема\\project\\dump"+QString::number(inc)+".txt";
        inc++;
        qDebug() << "Ошибка";
        QFile file(str_f);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(ba_all);
            file.close();
        }
    }
    double some = new_time / (1000 * 3600);
    qint8 hours = floor(some);
    hours %= 24;
    double day_of_week_1 = some / 24;
    int day_of_week = floor(day_of_week_1);
    QString d_o_w;
    switch (day_of_week) {
    case 1:
        d_o_w = "Понедельник";
        break;
    case 2:
        d_o_w = "Вторник";
        break;
    case 3:
        d_o_w = "Среда";
        break;
    case 4:
        d_o_w = "Четверг";
        break;
    case 5:
        d_o_w = "Пятница";
        break;
    case 6:
        d_o_w = "Суббота";
        break;
    case 0:
        d_o_w = "Воскресенье";
        QString str_f = "C:\\учеба\\навигационная микросхема\\project\\dump"+QString::number(inc)+".txt";
        inc++;
        qDebug() << "Ошибка";

        QFile file(str_f);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(ba_all);
            file.close();
        }
        break;
    }
    int min;
    min = floor(new_time / (60 * 1000)) - 24 * 60 * day_of_week - hours * 60;
    int sec;
    sec = floor(new_time / 1000) - 24 * 60 * 60 * day_of_week - hours * 60 * 60 - min * 60;
    ui->label_dayofweek->setText(d_o_w);
    QString format_time;
    format_time = QString::number(hours) + ":" + QString::number(min) + ":" + QString::number(sec);
    ui->label_hms->setText(format_time);
}

//Антенна NV2410
//Выбор антенны, SMA разъем, питание от микросхемы NV08C выбор какой - либо другой антенны??
//
void MainWindow::SerialRecieved()
{
    QByteArray ba_;
    ba_all = serial->readAll();                                             // Считываем данные с порта
    qDebug() << ba_all;
    int s = ba_all.size();
    int deleted_items = 0;
    for (int i = 2; i< s-3; i++)
    {
        qint8 bi = ba_all[i], bi1 = ba_all[i+1];
        if (bi == bi1 && bi == DLE)
        {
            ba_all.remove(i, 0);
            deleted_items++;
            for (int j = i; j < s; j++)
            {
                ba_all[j] = ba_all[j+1];
            }
        }
    }
    ba_all.resize(s-deleted_items);
    if (write_to_file || 1)
    {
        QString str_f = "C:\\учеба\\навигационная микросхема\\dump"+QString::number(inc)+".txt";
        QFile file(str_f);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(ba_all);
            file.close();
        }
        int i=0;
        QStringList strList;
        if ((file.exists())&&(file.open(QIODevice::ReadOnly)))
        {
            while(!file.atEnd())
            {
            strList << file.readLine();
            i++;
            }
            file.close();
        }
        if ((file.exists())&&(file.open(QIODevice::WriteOnly)))
        {
            strList.insert(i, ba_all);
            QTextStream stream(&file);
            foreach(QString s, strList)
            {
                stream<<s;
            }
            file.close();
        }
    }
}

void MainWindow::SerialSendMess(qint8 ID)
{
    // <DLE> - 1 байт, признак начала служебного кода (код 10h)
    // <ID> - идентификатор сообщения
    // <CRC> - 1 байт, признак начала онтрольной суммы (код FFh)
    // <KC> - 2 байта, контроьная сумма
    // <ETX> - 1 байт, признак конца сообщения (код 03h)
    // <DLE> <ID> data <DLE> <ETX>
    QByteArray mes;
    mes.resize(4);
    mes[0] = DLE;
    mes[1] = ID;
    mes[2] = DLE;
    mes[3] = ETX;
    serial->write(mes);
}

void MainWindow::SerialSendMess(qint8 ID, QByteArray data)
{
    QByteArray mes;
    int data_size = sizeof (data);
    mes.resize(4+data_size);
    mes[0] = DLE;
    mes[1] = ID;
    for (int i = 0; i < data_size; i++)
    {
        mes[2+i] = data[i];
    }
    mes[2 + data_size] = DLE;
    mes[3 + data_size] = ETX;
    serial->write(mes);
}

qint8 *MainWindow::getDATA(QByteArray recMess)
{
    int recMess_size = sizeof (recMess);
    qint8 retData[recMess_size - 4];
    for (int i = 0; i < recMess_size - 3; i++)
    {
        retData[i] = recMess[i + 2];
    }
    return retData;
}

void MainWindow::on_pushButton_clicked()
{
    QString str = ui->lineEdit->text();
    qint8 sec = str.toInt();
    if (sec!=NULL)
    {
        getVec(sec);
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(true);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    getVec(0x00);
    ui->pushButton->setEnabled(true);
}

void MainWindow::getVec(qint8 setSec)
{
    qint8 AskForVec = 0x27;            // ID запроса вектора состояний
    QByteArray eachSec;
    eachSec.resize(1);
    eachSec[0] = setSec;               // Выдавать ответ каждую секунду, для отмены 0x00
    SerialSendMess(AskForVec, eachSec);
}
int er=0, all_pack=0;
void MainWindow::convertInfo(QByteArray info)
{
    QString s;
    for(int a=0; a<ba_all.size(); a++)
    {
        s += QString("%1 ").arg(ba_all[a] & 0xFF, 2, 16, QChar('0'));
    }
    if (!ba_all[68+2] & 1)
    {
        er++;
        ui->label_3->setText("Неверно: " + QString::number(er));
    }
    all_pack++;
    ui->label_6->setText("Всего: " + QString::number(all_pack));
/*
    QString str_f = "C:\\учеба\\навигационная микросхема\\dump40.txt";
    QFile file(str_f + QString::number(it_files) + ".txt");
    file.open(QIODevice::ReadOnly);
    QByteArray info1;
    info1 = file.QIODevice::readAll();
    file.close();
    info = info1;*/
    vec new_vec;
    new_vec.lat = convBytesToDouble(info, 2);
    new_vec.lon = convBytesToDouble(info, 10);
    new_vec.height = convBytesToDouble(info, 18);
    new_vec.vlat = convBytesToDouble(info, 42);
    new_vec.vlat = convBytesToDouble(info, 50);
    new_vec.vlat = convBytesToDouble(info, 58);
    QByteArray data_out;
    data_out.resize(2);
    data_out[0] = info[40];
    data_out[1] = info[41];
    new_vec.week = *(reinterpret_cast<const qint16*>(data_out.constData()));
    new_vec.sko = convBytesToFloat(info, 26);
    new_vec.time = convBytesToLongDouble(info, 30);
    getTime(new_vec.time);
    freeLabels();
    ui->label_lat->setText(ui->label_lat->text() + "Широта: " + dtoa(new_vec.lat));
    ui->label_lon->setText(ui->label_lon->text() + "Долгота: " + dtoa(new_vec.lon));
    ui->label_height->setText(ui->label_height->text() + "Высота: " + dtoa(new_vec.height));
    ui->label_week->setText("Номер недели: " + QString::number(new_vec.week));
    ui->label_vlat->setText(ui->label_vlat->text() + "Скорость по широте: " + dtoa(new_vec.vlat));
    ui->label_vlon->setText(ui->label_vlon->text() + "Скорость по долготе: " + dtoa(new_vec.vlon));
    ui->label_vheight->setText(ui->label_vheight->text() + "Скорость по высоте: " + *dtoa(new_vec.vheight)); // Почему нельзя к строке нельзя прибавить char *
    ui->label_sko->setText(ui->label_sko->text() + "СКО: " + ftoa(new_vec.sko));
    ui->label_time->setText(ui->label_time->text() + "Время: " + ldtoa(new_vec.time));
}

char *MainWindow::dtoa(double dVal)
{
    static char buffer[_CVTBUFSIZE];
    memset(buffer, 0, _CVTBUFSIZE-1);
    int decimal, sign;
    char* fcvRes = _fcvt( dVal, 14, &decimal, &sign);
    if(sign)
        buffer[0] = '-';
    if(decimal <= 0){
        memset(buffer + sign, '0', 2 - decimal );
        buffer[sign + 1] = '.';
        memcpy(buffer + sign + 2 - decimal,fcvRes,strlen(fcvRes));
    }else{
        memcpy(buffer + sign, fcvRes, decimal);
        buffer[decimal + sign] = '.';
        memcpy(buffer + sign + decimal + 1, fcvRes + decimal, strlen(fcvRes)-decimal);
    }
    return buffer;
}

char *MainWindow::ldtoa(long double dVal)
{
    static char buffer[_CVTBUFSIZE];
    memset(buffer, 0, _CVTBUFSIZE-1);
    int decimal, sign;
    char* fcvRes = _fcvt( dVal, 14, &decimal, &sign);
    if(sign)
        buffer[0] = '-';
    if(decimal <= 0){
        memset(buffer + sign, '0', 2 - decimal );
        buffer[sign + 1] = '.';
        memcpy(buffer + sign + 2 - decimal,fcvRes,strlen(fcvRes));
    }else{
        memcpy(buffer + sign, fcvRes, decimal);
        buffer[decimal + sign] = '.';
        memcpy(buffer + sign + decimal + 1, fcvRes + decimal, strlen(fcvRes)-decimal);
    }
    return buffer;
}

char *MainWindow::ftoa(float fVal)
{
    static char buffer[_CVTBUFSIZE];
    memset(buffer, 0, _CVTBUFSIZE-1);
    int decimal, sign;
    char* fcvRes = _fcvt( fVal, 17, &decimal, &sign);
    if(sign)
        buffer[0] = '-';
    if(decimal <= 0){
        memset(buffer + sign, '0', 2 - decimal );
        buffer[sign + 1] = '.';
        memcpy(buffer + sign + 2 - decimal,fcvRes,strlen(fcvRes));
    }else{
        memcpy(buffer + sign, fcvRes, decimal);
        buffer[decimal + sign] = '.';
        memcpy(buffer + sign + decimal + 1, fcvRes + decimal, strlen(fcvRes)-decimal);
    }
    return buffer;
}

double MainWindow::convBytesToDouble(QByteArray data_in, qint8 start_with)
{
    int i = 0;
    QByteArray data_out;
    data_out.resize(8);
    for (;i<8;i++)
    {
        data_out[i] = data_in[start_with + i];
    }
    double d_data_out = *(reinterpret_cast<const double*>(data_out.constData()));
    return d_data_out;
}

long double MainWindow::convBytesToLongDouble(QByteArray data_in, qint8 start_with)
{
    int i = 0;
    QByteArray data_out;
    data_out.resize(10);
    for (;i<10;i++)
    {
        data_out[i] = data_in[start_with + i];
    }
    long double d_data_out = *(reinterpret_cast<const long double*>(data_out.constData()));
    return d_data_out;
}

float MainWindow::convBytesToFloat(QByteArray data_in, qint8 start_with)
{
    int i = 0;
    QByteArray data_out;
    data_out.resize(4);
    for (;i<4;i++)
    {
        data_out[i] = data_in[start_with + i];
    }
    float d_data_out = *(reinterpret_cast<const float*>(data_out.constData()));
    return d_data_out;
}

void MainWindow::freeLabels()
{
    ui->label_lat->setText("");
    ui->label_lon->setText("");
    ui->label_vlat->setText("");
    ui->label_vlon->setText("");
    ui->label_height->setText("");
    ui->label_vheight->setText("");
    ui->label_week->setText("");
    ui->label_sko->setText("");
    ui->label_time->setText("");
}

void MainWindow::on_pushButton_Start_clicked()
{
    qint8 checkConnecion = 0x26;       // Запрос подтверждения связи, ответ - 54h
    qint8 stopAnsw = 0x0e;             // Запрет на выдачу
    if (!iConnect)
    {

        QString portName = ui->comboBox->currentText();
        serial->setPortName(portName);                        // Выбираем COM-порт
        serial->open(QIODevice::ReadWrite);                 // Открываем порт в режиме чтения-записи
        //SerialSendMess(stopAnsw);
        //SerialSendMess(checkConnecion);    // Проверка связи
        if(iConnect)
            ui->pushButton_Start->setText("Stop");
    }
    else {
        SerialSendMess(stopAnsw);
        serial->close();
        ui->pushButton_Start->setText("Start");
        iConnect = false;
    }
}
void MainWindow::open_file()
{
    qDebug() << "work";
}
/*
 Удалить двойной байт 01 при приеме ++++
 Во время работы надпись на кнопке start заменить на стоп
 QDateTime для работы с форматом времени и днём недели
 Эпохи
 Leap seconds
 */
