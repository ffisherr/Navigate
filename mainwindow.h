#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void SerialRecieved();
    void StartWork();
    void SerialSendMess(qint8 ID);
    void SerialSendMess(qint8 ID, QByteArray data);
    qint8 *getDATA(QByteArray recMess);
    void on_pushButton_clicked();
    void getVec(qint8 setSec);
    void on_pushButton_2_clicked();
    void convertInfo(QByteArray info);
    char *dtoa(double dVal);
    char *ftoa(float fVal);
    char *ldtoa(long double dVal);
    double convBytesToDouble(QByteArray data_in, qint8 start_with);
    long double convBytesToLongDouble(QByteArray data_in, qint8 start_with);
    void freeLabels();
    float convBytesToFloat(QByteArray data_in, qint8 start_with);
    void getTime(long double time);
    void checkAnswer();
    void on_pushButton_Start_clicked();
    void open_file();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
