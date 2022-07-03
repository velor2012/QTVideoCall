#ifndef MYBASEQTHREAD_H
#define MYBASEQTHREAD_H
#include <QThread>
class MyBaseQThread : public QThread
{
    Q_OBJECT

public:
    MyBaseQThread(QObject* parent):QThread(parent), parent(parent), finish(false), init(false){
        connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    }
    ~MyBaseQThread() = default;
    virtual void stop() {
        finish = true;
    }
    bool init = false;

protected:
    int handle = -1;
    QObject* parent;
    volatile bool finish = false;
    virtual void task() {};
    virtual void run() {
        while (!finish) {
            task();
        }
    }
};
#endif // MYBASEQTHREAD_H
