#ifndef MYQUEUE_H
#define MYQUEUE_H
#include <queue>
#include <mutex>
#include <condition_variable>
#define MAX_QUE_SIZE 100
//#include <QtCore>
template <typename T, int type = 0>
class MyQueue
{
private:
    std::queue<T> q;
    std::mutex mtx;
    std::condition_variable con_v;
public:
    MyQueue(){
    }
    std::queue<T>& get(){
        return q;
    }
    T pop_front(){
        //取数据
        std::unique_lock<std::mutex> lock(mtx);
        con_v.wait_for(lock, std::chrono::seconds(5), [this] {
            return !this->q.empty();
        });
        if (this->q.empty()) return nullptr;
//        qDebug() << "in pop_front, remain: " +  QString::number(this->q.size());
        T pcm_data = this->q.front();
        this->q.pop();
        return pcm_data;
    }

    void push(T t){
        std::unique_lock<std::mutex> lock(mtx);
        if(this->q.size() > MAX_QUE_SIZE){
            auto t1 = this->q.front();
            if(type == 1) free(t1);
            else delete t1;
            this->q.pop();
        }
        this->q.push(t);
//        qDebug() << "in push" + QString::number(this->q.size());
        con_v.notify_all();

    }

    void clear_use_free(){
        std::unique_lock<std::mutex> lock(mtx);
        while(!this->q.empty()){
            auto pkt = this->q.front();
            this->q.pop();
            free(pkt);
        }
    }

    void clear_use_delete(){
        std::unique_lock<std::mutex> lock(mtx);
        while(!this->q.empty()){
            auto pkt = this->q.front();
            this->q.pop();
            delete(pkt);
        }
    }
};

#endif // MYQUEUE_H
