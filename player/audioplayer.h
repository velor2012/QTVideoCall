#ifndef AUDIOSTARTTHREAD_H
#define AUDIOSTARTTHREAD_H

#include <QObject>
#include <QThread>

class AudioPlayeThread : public QThread
{
    Q_OBJECT
public:
    explicit AudioPlayeThread(QObject *parent = nullptr);
    ~AudioPlayeThread();

private:
    void run() override;

signals:

};

#endif // AUDIOSTARTTHREAD_H
