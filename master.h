#ifndef MASTER_H
#define MASTER_H

#define MAX_COUNT_THREAD 30

#include <QObject>
#include <QList>
#include <QThread>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "worker.h"

struct counters {
    int totalLines;
    int compleatedLines;
    int badRequest;
    int errors;
    void clear() {
        this->totalLines =0;
        this->compleatedLines =0;
        this->badRequest =0;
        this->errors =0;
    }
};

class Master : public QObject
{
    Q_OBJECT
public:
    explicit Master(QObject *parent = nullptr);

private:
    counters cnt;              //Счетчики
    competitor *pcmp;          //Укащатель на конкурента
    lineData   *pline;         //Указатель на наш товар и список товаров конкурентов
    parse_data *pd;            //Данные для парсинга
    bool doStop;               //Флаг остановки приложения
    bool useProxy;             //Использовать прокси
    int currItemIndex;         //Индекс текущего парсинга
    int countThreads;         //Кол. потоков
    QList <Worker*> *thrPool;  //Пул рабочих потоков


    void create_workers();  //Создать рабочие потоки
    void stop_and_delete_thread(Worker *W);
    void emit_task(QObject *obj);
    void emit_free_thread(QObject *obj);
    bool prepareData(parse_data *ppd);
    bool prepareAutoScanerData(parse_data *ppd);
    QString removeFirstEnd(QString str);
    void saveParserData();
    void saveHtmlParserData();
    
signals:
    void showBar(QString);
    void showCurrentCountThreads(int);
    void wakeUpAllWorkers();
    void init_workers();
    void enabledStart();
    void runTask();
    void free_thread();
    void showCounters(counters *);
    void setMaxValueProgressBar(int val);
    void plusProgressBar();
    void clearProgressBar();
    void addStatusToTableWidget(int, QString, QString, QString);
    void showMsgBox(QString);
    
public slots:
    void initMaster(int countThreads, parse_data *pd, bool useProxy);
    void threadReadyQuit();
    void setTask(TInfo* info);
    void plusError();
    void plusBadRequest();
    void stopWork();
    
};

#endif // MASTER_H
