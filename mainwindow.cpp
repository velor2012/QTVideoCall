#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tool.h"
#include "Config.h"
MySessionManager* MSM = nullptr;
extern uint8_t volume;
extern Config* cfg;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sw = new SettingWindow();
    apthread = new AudioPlayeThread(this);
    apthread->start();
    vpencthread = new VideoPlayerEnc(this);
    vpencthread->start();
    vpdecthread = new VideoPlayerDec(this);
    vpdecthread->start();
    MSM = new MySessionManager(this);
    setTitle("未开始会话");

    rgb_dis_buf = (unsigned char*)malloc(MAX_FRAME_SIZE * 2 * sizeof(char));
    rgb_dec_buf = (unsigned char*)malloc(MAX_FRAME_SIZE * 2 * sizeof(char));

    cur_enc_lb_w = ((ui->label_2->width() - 1) >> 1) << 1;
    cur_enc_lb_h = ((ui->label_2->height() - 1) >> 1) << 1;
    ui->label_2->resize(cur_enc_lb_w, cur_enc_lb_h);

    cur_dec_lb_w = ((ui->label->width() - 1) >> 1) << 1;
    cur_dec_lb_h = ((ui->label->height() - 1) >> 1) << 1;
    ui->label->resize(cur_dec_lb_w, cur_dec_lb_h);
}
void MainWindow::closeEvent(QCloseEvent *e){
}
MainWindow::~MainWindow()
{
    vpencthread->stop();
    vpencthread->wait();
    vpdecthread->stop();
    vpdecthread->wait();
    delete ui;
}


void MainWindow::on_settingButton_clicked()
{
   if(MSM && MSM->getStat() != 0) return;
   sw->show();
}


void MainWindow::on_startButton_clicked()
{
    if(MSM->getStat() == 0){
        auto cfg = Config::GetInstance();
        cfg->target_ip = ui->targetIPEdit->text();
        cfg->target_port = ui->targetPortSpinBox->value();
        MSM->call(cfg->target_ip, cfg->target_port);
    }else{
        MSM->hupSession();
    }
}

void MainWindow::setTitle(QString title){
    setWindowTitle(title); //设置标题栏名称
}

void MainWindow::updateStartButtion(){
    QString t = MSM->getStat() == 0 ? "呼叫" : "挂断";
    ui->startButton->setText(t);
}

void MainWindow::show_encode_buf() {
    //ui->EncOpenGLWidget->DisplayVideoFrame(yuv_dis_buf, encodeConfig->width, encodeConfig->height);
    QImage tmpImg((uchar*)rgb_dis_buf, ui->label_2->width(), ui->label_2->height(), QImage::Format_RGB32);

    ui->label_2->setPixmap(QPixmap::fromImage(tmpImg));

}

void MainWindow::show_decode_buf() {
    //ui->DecOpenGLWidget->DisplayVideoFrame(yuv_dec_buf, imgDecWidth_new, imgDecHeight_new );
    QImage tmpImg((uchar*)rgb_dec_buf, ui->label->width(), ui->label->height(), QImage::Format_RGB32);
    ui->label->setPixmap(QPixmap::fromImage(tmpImg));
}

void MainWindow::on_verticalSlider_sliderMoved(int position)
{
    volume = position;
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{
    cfg->mute = arg1;
}

