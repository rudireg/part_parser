#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include "partparser.h"

class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int m_threadID READ threadID WRITE setThreadID)

public:
    explicit Worker(QObject *parent = nullptr);
    void setThreadID(int i) {this->m_threadID = i;}
    int  threadID() {return this->m_threadID;}
    
private:
    partParser *pParser;
    TInfo *info;
    int m_threadID; //Идентификатор потока

signals:
    void iAmReady(TInfo*);
    void iAmReadyQuit();
    void addStatusToTableWidget(int, QString, QString, QString);
    void plusError();
    void plusBadRequest();
    
public slots:
    void init_workers();
    void wakeUpAllWorkers();
    void free_thread();
    void do_task();
    void addStatus(QString article, QString status, QString color);

};

#endif // WORKER_H
