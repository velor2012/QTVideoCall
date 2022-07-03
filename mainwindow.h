#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingwindow.h"
#include "player/audioplayer.h"
#include "mysessionmanager.h"
#include "player/videoplayerEnc.h"
#include "player/videoplayerdec.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setTitle(QString title);
    void updateStartButtion();
    Q_INVOKABLE void show_encode_buf();
    Q_INVOKABLE void show_decode_buf();
    unsigned char* rgb_dis_buf;  //原始视频rgb
    unsigned char* rgb_dec_buf;  //目的端解码视频rgb
    int cur_enc_lb_w;
    int cur_enc_lb_h;
    int cur_dec_lb_w;
    int cur_dec_lb_h;
    AudioPlayeThread* apthread;
private slots:
    void closeEvent(QCloseEvent *e);
    void on_settingButton_clicked();

    void on_startButton_clicked();

    void on_verticalSlider_sliderMoved(int position);

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    SettingWindow* sw;

    VideoPlayerEnc* vpencthread;
    VideoPlayerDec* vpdecthread;
};
#endif // MAINWINDOW_H
