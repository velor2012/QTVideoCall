#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "Config.h"
#include <QMediaPlayer>   //第1句
#include "private/qvideoframeconversionhelper_p.h"
#include "cammicloader.h"
#include "tool.h"
#include "player/audioplayer.h"
#include "mysessionmanager.h"
#include <QMessageBox>
constexpr int MAX_PCM_QUE_SIZE = 100;
extern Config* cfg;
extern MySessionManager* MSM;
SettingWindow* sw;

//采集到的视频图像回调
LRESULT CALLBACK VideoCaptureCallback(uint8_t* buff, INT64 lTimeStamp)
{
    if(!sw) return 0;
    YUV2RGB(buff, cfg->w, cfg->h, sw->video_buff, sw->cur_w_l, sw->cur_h_l);
    sw->show_camera();
    return 0;
}

////采集到的音频数据回调
LRESULT CALLBACK AudioCaptureCallback(uint8_t* buff, int audio_out_buffer_size, INT64 lTimeStamp)
{
    if(!sw) return 0;
//    int level = ComputeLevel((const int16_t*)buff, audio_out_buffer_size / 2);
    sw->mic_level = SimpleCalculate_DB((short*)buff, 882);
   char* t((char*)malloc(audio_out_buffer_size));
    memcpy(t, buff, audio_out_buffer_size);

    cfg->pcm_que.push(t);

    cfg->pcm_pkt_l = audio_out_buffer_size;

    return 0;
}

SettingWindow::SettingWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingWindow),
    m_pushTimer(new QTimer(this))
{
\
    ui->setupUi(this);
    sw = this;
    cur_h_l = ui->label_2->width();
    cur_w_l = ui->label_2->width();
    ui->micVolumeBar->setMaximum(100);

    video_buff = (uint8_t*)malloc(cfg->w * cfg->h * 3);
    for(auto &it : cfg->videolist){
       ui->videoDeviceComboBox->addItem(it);
    }

    for(auto &it : cfg->audiolist){
       ui->audiodeviceComboBox->addItem(it);
    }

    ui->portSpinBox->setValue(cfg->local_port);
    save();

}

SettingWindow::~SettingWindow()
{
    delete ui;
    CamMicLoader*& camloader = cfg->CML;
    if(camloader){
        camloader->CloseInputStream();
        delete camloader;
        camloader = nullptr;
        m_pushTimer->stop();
        m_pushTimer->disconnect();
        delete m_pushTimer;
    }
}

void SettingWindow::closeEvent(QCloseEvent *e){
    CamMicLoader*& camloader = cfg->CML;
    if(camloader){
        camloader->CloseInputStream();
        delete camloader;
        camloader = nullptr;
        m_pushTimer->stop();
        m_pushTimer->disconnect();
    }
    save();
}
void SettingWindow::save(){
    int t_p = cfg->local_port;
    cfg->local_port = ui->portSpinBox->value();
    cfg->fps = ui->fpsSpinBox->value();
    if(cfg->local_port != t_p){
        bool res = MSM->updateUDP();
        if(!res){
            QMessageBox::warning(nullptr, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("update udp port fail"));
            ui->portSpinBox->setValue(t_p);
        }else{
            cfg->local_port = ui->portSpinBox->value();
        }

    }

    auto t = ui->videoQualityComboBox->currentIndex();
    cfg->cur_cam = ui->videoDeviceComboBox->currentText().toStdWString();
    cfg->cur_mic = ui->audiodeviceComboBox->currentText().toStdWString();
    switch (t) {
    case 0:
        cfg->w = 1920;
        cfg->h = 1080;
        break;
    case 1:
        cfg->w = 1280;
        cfg->h = 768;
        break;
    case 2:
        cfg->w = 1440;
        cfg->h = 900;
        break;
    case 3:
    default:
        cfg->w = 600;
        cfg->h = 400;
    }

    auto t2 = ui->bitrateComboBox->currentIndex();
    switch (t2) {
    case 0:
        cfg->bitrate = 0;
        break;
    case 1:
        cfg->bitrate = 2048 * 1000;
        break;
    case 2:
        cfg->bitrate = 1024 * 1000;
        break;
    case 3:
        cfg->bitrate = 512 * 1000;
        break;
    case 4:
        cfg->bitrate = 256 * 1000;
        break;
    default:
        cfg->bitrate = 0;
    }
}

void SettingWindow::on_testVideoButton_clicked()
{
    CamMicLoader*& camloader = cfg->CML;
    if(!camloader){
        camloader = new CamMicLoader(cfg->w,cfg->h);
        auto v_device = ui->videoDeviceComboBox->currentText().toStdWString();
        auto a_device = ui->audiodeviceComboBox->currentText().toStdWString();
        qDebug() << v_device;
        qDebug() << a_device;
        qDebug() << ui->videoDeviceComboBox->currentText();
        int tt = camloader->initLoader(v_device, a_device);

        camloader->SetVideoCaptureCB(VideoCaptureCallback);
        camloader->SetAudioCaptureCB(AudioCaptureCallback);
        camloader->StartCapture();

    }else{
        camloader->CloseInputStream();
        delete camloader;
        camloader = nullptr;
    }

}

void SettingWindow::show_camera()
{
    QImage tmpImg((uchar*)video_buff, cur_w_l, cur_h_l, QImage::Format_RGB32);
    ui->label_2->setPixmap(QPixmap::fromImage(tmpImg));
}

void SettingWindow::update_mic_level()
{
    ui->micVolumeBar->setValue(mic_level);
}


void SettingWindow::on_testAudioButton_clicked()
{
    static bool open = false;
    open = !open;

    m_pushTimer->stop();
    m_pushTimer->disconnect();
    if(!open) return;
    connect(m_pushTimer, &QTimer::timeout, [this]() {
        qDebug() <<"int";
        update_mic_level();
    });
    m_pushTimer->start(20);
}

