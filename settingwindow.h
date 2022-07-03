#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QWidget>
#include <QtMultimedia/QCamera>
#include <QMediaCaptureSession>
#include <QtMultimedia/QVideoSink>
#include <QCloseEvent>
#include "cammicloader.h"
#include <QTimer>

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();
    uint8_t* video_buff;
    int cur_h_l;
    int cur_w_l;
    volatile int mic_level;
    void show_camera();
    void update_mic_level();
    void save();
    QTimer* m_pushTimer;

private slots:
    void closeEvent(QCloseEvent *e);
    void on_testVideoButton_clicked();
    void on_testAudioButton_clicked();

private:
    Ui::SettingWindow *ui;

};

#endif // SETTINGWINDOW_H
