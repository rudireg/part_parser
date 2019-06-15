#include "worker.h"
//------------------------------------------------------
Worker::Worker(QObject *parent) :
    QObject(parent)
{
    this->info    = nullptr;
    this->pParser = nullptr;
}
//------------------------------------------------------
//Высвобождение памяти Worker
void Worker::free_thread()
{
    if(this->info) {
        this->info->clear();
        delete this->info;
    }
   if(this->pParser != nullptr) delete this->pParser;
   emit iAmReadyQuit();
}
//------------------------------------------------------
void Worker::wakeUpAllWorkers()
{
    this->info->clear();
    emit iAmReady(this->info);
}
//------------------------------------------------------
void Worker::init_workers()
{
    this->info    = new TInfo;
    this->pParser = new partParser();
    connect(this->pParser,SIGNAL(plusError()),this,SIGNAL(plusError()));
    connect(this->pParser,SIGNAL(plusBadRequest()),this,SIGNAL(plusBadRequest()));
    connect(this->pParser,SIGNAL(addStatus(QString,QString,QString)),
            this,SLOT(addStatus(QString,QString,QString)));
}
//------------------------------------------------------
void Worker::do_task()
{
    switch(this->info->task)
    {
    //Парсим
    case wtPARSER:
        switch (this->pParser->doParser(this->info->ld)) {
        case PARSE_OK:
            this->info->status = wsSUCCESS;
            break;
        case PARSE_ERROR:
            this->info->status = wsERROR;
            break;
        case PARSE_BADREQUEST:
            this->info->status = wsBADREQUEST;
            break;
        default:
            this->info->status = wsERROR;
        }
    }

    emit iAmReady(this->info);
}
//-------------------------------------------------------
//пересылает сигнал с добавлением идентификатора потока
void Worker::addStatus(QString article, QString status, QString color)
{
    emit addStatusToTableWidget(this->m_threadID, article, status, color);
}
//-------------------------------------------------------

