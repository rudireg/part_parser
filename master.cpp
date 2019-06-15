#include "master.h"
//-------------------------------------------------------
Master::Master(QObject *parent) :
    QObject(parent)
{
    this->thrPool = new QList <Worker*>; //Высвобождается тут: stop_and_delete_thread
}
//-------------------------------------------------------
void Master::initMaster(int countThreads, parse_data *pd, bool useProxy)
{
    emit showBar("Начало работы");
    if(countThreads < 1) this->countThreads =1;
    else if(countThreads > MAX_COUNT_THREAD) this->countThreads = MAX_COUNT_THREAD;
    else this->countThreads = countThreads;
    //Показываем юзеру сколько потоков запущено на самом деле
    emit showCurrentCountThreads(this->countThreads);
    //Нужно ли использовать прокси
    this->useProxy = useProxy;
    //Запомнить данные для парсинга
    this->pd = pd;
    this->currItemIndex =0;
    this->doStop = false;
    this->cnt.clear();

    //Подгатавливаем рабочую информацию для парсинга
    //Открываем файл и парсим данные, которые необходимо обработать в дальнейшем
    if(pd->useAutoScanerParser == true){
        if(!this->prepareAutoScanerData(this->pd)) {
                QMessageBox::information(nullptr,"Error","Error prepare *.csv файл");
                emit showBar("Ошибка обработки *.csv файла");
                return;
        }
    }else if(!this->prepareData(this->pd)) {
        QMessageBox::information(nullptr,"Error","Error prepare *.csv файл");
        emit showBar("Ошибка обработки *.csv файла");
        return;
    }
    emit showCounters(&this->cnt);
    emit clearProgressBar();
    emit setMaxValueProgressBar(this->cnt.totalLines);

    //Создаем рабочие потоки
    this->create_workers();

    //Пробуждаем рабочие потоки
    emit showBar("Пробуждаем потоки");
    emit wakeUpAllWorkers();
}
//-------------------------------------------------------
//Создать рабочие потоки
void Master::create_workers()
{
    if(!this->thrPool->isEmpty()) this->thrPool->clear();
    Worker *worker;
    QThread *thread;
    for(int i=0; i<this->countThreads; ++i){
        emit showBar(QString("Создание потока: %1").arg(i+1));
        worker = new Worker;
        thread = new QThread;
        thread->setObjectName(QString("worker_%1").arg(i));
        this->thrPool->append(worker);

        connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
        connect(this,SIGNAL(init_workers()),worker,SLOT(init_workers()));
        connect(this,SIGNAL(wakeUpAllWorkers()),worker,SLOT(wakeUpAllWorkers()));
        connect(worker,SIGNAL(iAmReady(TInfo*)),this,SLOT(setTask(TInfo*)));
        connect(worker,SIGNAL(iAmReadyQuit()),this,SLOT(threadReadyQuit()));
        connect(worker,SIGNAL(addStatusToTableWidget(int,QString,QString,QString)),
                this,SIGNAL(addStatusToTableWidget(int,QString,QString,QString)));
        connect(worker,SIGNAL(plusError()),this,SLOT(plusError()));
        connect(worker,SIGNAL(plusBadRequest()),this,SLOT(plusBadRequest()));

        worker->setThreadID(i);
        worker->moveToThread(thread);
        thread->start();
    };
    emit init_workers(); //Тут идет инициализация мамбы
}
//-------------------------------------------------------
void Master::setTask(TInfo* info)
{
    Worker  *W = static_cast<Worker*>(this->sender());
    QThread *T = W->thread();
    if(!W || !T) {
        emit showBar("Критическая ошибка: W == NULL || T == NULL");
        QMessageBox::information(nullptr,"Error","Critical error: W == NULL || T == NULL");
        info->clear();
        return;
    }
    emit showBar("Работа парсинга ...");
    switch(info->status) {
    case wsSUCCESS:
        this->cnt.compleatedLines ++;
        emit plusProgressBar();
        break;
    case wsERROR:
        this->cnt.errors ++;
        emit plusProgressBar();
        break;
    case wsBADREQUEST:
        this->cnt.badRequest ++;
        emit plusProgressBar();
        break;
    }
    //Показать счетчики
    emit showCounters(&this->cnt);
    //**************************************************************
    //**************** Назначаем новое задание *********************
    //**************************************************************
    if(this->doStop) {
        this->stop_and_delete_thread(W);
        return;
    }
    info->clear();
    if(this->currItemIndex < this->pd->listData.size()) {
        //Берем данные для парсинга
        info->ld = this->pd->listData.at(this->currItemIndex++);
        info->ld->useAutoScaners = this->pd->useAutoScanerParser; //тип работы
    } else {
        this->doStop = true;
        this->stop_and_delete_thread(W);
        return;
    }
    info->task = wtPARSER;
    this->emit_task(W);
}
//-------------------------------------------------------
//А вот тут запуск процесса: высвобождения и удаления дочернего потока
void Master::stop_and_delete_thread(Worker *W)
{
   for(int i=0; i < this->thrPool->count(); ++i)
   {
       if(this->thrPool->at(i) == W) {
           emit showBar(QString("Останавливаем поток: %1").arg(W->threadID()));
           this->thrPool->removeAt(i);
           this->emit_free_thread(W);
           break;
       }
   };

   if(this->thrPool->isEmpty()) {
       emit showBar("Все потоки остановлены");
       emit showBar("Сохраняем отчет");
       this->saveParserData();
       emit showBar("Отчет сохранен");
       emit enabledStart();
       emit showMsgBox("Work is done");
   }
}
//-------------------------------------------------------
void Master::emit_task(QObject *obj)
{
    if(this->doStop) return;
    connect(this,SIGNAL(runTask()),obj,SLOT(do_task()));
    emit runTask();
    disconnect(this,SIGNAL(runTask()),obj,SLOT(do_task()));
}
//-------------------------------------------------------
void Master::emit_free_thread(QObject *obj)
{
    connect(this,SIGNAL(free_thread()),obj,SLOT(free_thread()));
    emit free_thread();
    disconnect(this,SIGNAL(free_thread()),obj,SLOT(free_thread()));
}
//-------------------------------------------------------
void Master::threadReadyQuit()
{
    Worker  *W = static_cast<Worker*>(this->sender());
    QThread *T = W->thread();

    W->deleteLater();
    T->quit();
}
//-------------------------------------------------------
//Подготовить данные для парсинга AutoScaner.Ru
bool Master::prepareAutoScanerData(parse_data *ppd)
{
    if(!ppd->dirFile.contains(".csv")) return false;
    QFile file(ppd->dirFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString str;
    QStringList list;
    //Парсим заголовок (URL доменов что должны будут спарсены), пропуская первый: Наименование Автосканеры.RU
    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("Windows-1251"));
    str = in.readLine();
    str.remove(QChar('\n'));
    list = str.split(ppd->inopt.field_separator);
    if(list.count() < 2) {
        file.close();
        return false;
    }
    int x=0;
    foreach(QString lst, list) {
        //Пропускаем первый: Наименование Автосканеры.RU
        if(++x <= 1) continue;
        ppd->domains.append(this->removeFirstEnd(lst));
    };
    if(ppd->domains.isEmpty()) {
        file.close();
        return false;
    }
    //Парсим тело *.csv
    while(!in.atEnd()) {
        str = in.readLine();
        str.remove(QChar('\n'));
        list = str.split(ppd->inopt.field_separator);
        if(list.count() < 2) continue;
        this->cnt.totalLines ++; //Увеличиваем кол. артикулов для счетчика
        lineData *newLine = new lineData;
        newLine->clear();
        int step =0;
        foreach(QString lst, list) {
            step ++;
            if(step == 1)      //Наименование нашего товара
                newLine->manufacture = this->removeFirstEnd(lst);
            else {
                competitor * comp = new competitor;     //Создаем конкурента
                comp->clear();
                comp->url         = ppd->domains.at(step-2); //Получаем URL конкурента
                comp->articule    = this->removeFirstEnd(lst);
                if(comp->articule.isEmpty() || !comp->articule.contains("http")) comp->isHave = false;
                else comp->isHave = true;
                newLine->comp.append(comp);
            }
        };
        ppd->listData.append(newLine);
    };

    file.close();
    return true;
}
//-------------------------------------------------------
//Подготовить данные для парсинга
bool Master::prepareData(parse_data *ppd)
{
    if(!ppd->dirFile.contains(".csv")) return false;
    QFile file(ppd->dirFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString str;
    QStringList list;
    //Парсим заголовок (URL доменов что должны будут спарсены), пропуская первые три: Наименование, артикул, цена
    QByteArray line = file.readLine();
    str = QString::fromUtf8(line);
    str.remove(QChar('\n'));
    list = str.split(ppd->inopt.field_separator);
    if(list.count() < 3) {
        file.close();
        return false;
    }
    int x=0;
    foreach(QString lst, list) {
        //Пропускаем первые два: Наименование, артикул
        if(++x <= 2) continue;
        ppd->domains.append(this->removeFirstEnd(lst));
    };
    if(ppd->domains.isEmpty()) {
        file.close();
        return false;
    }

    //Парсим тело *.csv
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        str = QString::fromLatin1(line);
        str.remove(QChar('\n'));
        list = str.split(ppd->inopt.field_separator);
        if(list.count() < 3) continue;
        this->cnt.totalLines ++; //Увеличиваем кол. артикулов для счетчика
        lineData *newLine = new lineData;
        newLine->clear();
        int step =0;
        foreach(QString lst, list) {
            step ++;
            if(step == 1)      //Наименование нашего товара
                newLine->manufacture = this->removeFirstEnd(lst);
            else if(step == 2) //Артикул нашего товара
                newLine->articule    = this->removeFirstEnd(lst);
            else {
                competitor * comp = new competitor;     //Создаем конкурента
                comp->clear();
                comp->url         = ppd->domains.at(step-3); //Получаем URL конкурента
                comp->articule    = this->removeFirstEnd(lst);
                if(comp->articule.isEmpty()) comp->isHave = false;
                else comp->isHave = true;
                newLine->comp.append(comp);
            }
        };
        ppd->listData.append(newLine);
    };

    file.close();
    return true;
}
//-------------------------------------------------------
//Удалить первый и послдний символы
QString Master::removeFirstEnd(QString str)
{
    if(str.size() < 3) return str;
    QChar ch = this->pd->inopt.text_separator[0];
    if(str[0] != ch || str[str.length()-1] != ch)
       return str;

    QString tmp = str.remove(0,1);
    tmp.chop(1);
    return tmp;
}
//-------------------------------------------------------
void Master::plusError()
{
    this->cnt.errors ++;
    emit showCounters(&this->cnt);
}
//-------------------------------------------------------
void Master::plusBadRequest()
{
    this->cnt.badRequest ++;
    emit showCounters(&this->cnt);
}
//-------------------------------------------------------
void Master::stopWork()
{
    this->doStop = true;
}
//-------------------------------------------------------
//Сохранить данные
void Master::saveParserData()
{
    QString val;
    QFile filename("DATA/result.csv");
    filename.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&filename);
    out.setCodec(QTextCodec::codecForName("Windows-1251"));

    if(this->pd->useAutoScanerParser == true){
        this->saveHtmlParserData(); //Сохраняем в формате HTML
        //Заголовоки
        out << QString("%1").arg(QString::fromUtf8("Наименование Автосканеры.RU"));
        //Заголовки сайтов
        for(int i=0; i < this->pd->domains.size(); ++i) {
            out << QString(";%1").arg(this->pd->domains.at(i));
        }
        out << QString("%1").arg("\n");
        for(int i=0; i < this->pd->listData.size(); ++i) {
            this->pline = this->pd->listData.at(i);
            //Наши данные
            out << QString("%1").arg(this->pline->manufacture);
            for(int x=0; x < this->pline->comp.size(); ++x) {
                this->pcmp = this->pline->comp.at(x);
                out << QString(";%1").arg(QString().setNum(this->pcmp->price));
            };
            out << QString("%1").arg("\n");
        }
    }else{
        //Заголовоки
        out << QString("%1;%2;%3").arg(QString::fromUtf8("Наименование")).arg(QString::fromUtf8("Артикул")).arg("www.car-tool.ru");
        //Заголовки сайтов
        for(int i=0; i < this->pd->domains.size(); ++i) {
            out << QString(";%1").arg(this->pd->domains.at(i));
        }
        out << QString("%1").arg("\n");
        for(int i=0; i < this->pd->listData.size(); ++i) {
            this->pline = this->pd->listData.at(i);
            //Наши данные
            if(static_cast<int>(this->pline->price) == -1) {
                val = QString("%1").arg("ERROR");
            }else{
                val = QString("%1").arg(QString().setNum(this->pline->price));
            }
            out << QString("%1;%2;%3").arg(this->pline->manufacture).arg(this->pline->articule).arg(val);
            for(int x=0; x < this->pline->comp.size(); ++x) {
                this->pcmp = this->pline->comp.at(x);
                //if(this->pcmp->articule.isEmpty()) this->pcmp->articule = " ";
                out << QString(";%1").arg(QString().setNum(this->pcmp->price));
            };
            out << QString("%1").arg("\n");
        }
    }
    filename.close();
}
//-------------------------------------------------------
//Сохранить данные в формате html
void Master::saveHtmlParserData()
{
    QString color;
    QFile filename("DATA/HTML_result.html");
    filename.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&filename);
    out.setCodec(QTextCodec::codecForName("Windows-1251"));

    //****************************
    out << "<html>"
           "<head>"
           "</head>"
           "<body>"
           "<table cellspacing='2' border='1' cellpadding='5'>";

    //Заголовоки
    out << "<tr>";
    out << QString("<td>%1</td>").arg(QString::fromUtf8("Наименование Автосканеры.RU"));
    for(int i=0; i < this->pd->domains.size(); ++i)
        out << QString("<td>%1</td>").arg(this->pd->domains.at(i));
    out << "</tr>";

    //Данные
    float sourcePrice =0;
    for(int i=0; i < this->pd->listData.size(); ++i) {
        this->pline = this->pd->listData.at(i);
        out << "<tr>";
        //Наши данные
        out << QString("<td>%1</td>").arg(this->pline->manufacture);
        for(int x=0; x < this->pline->comp.size(); ++x) {
            this->pcmp = this->pline->comp.at(x);
            if(x == 0) sourcePrice = this->pcmp->price;
            //Если нет сссылки
            if(this->pcmp->isHave == false){
                out << "<td></td>";
                continue;
            }
            if(this->pcmp->price >= sourcePrice) color = "green";
            else color = "red";
           out << QString("<td><a target='blanc' style='font-weight:600; color:%1' href='%2'>%3</a></td>")
                  .arg(color).arg(this->pcmp->articule).arg(QString().setNum(this->pcmp->price));
        };
        out << "</tr>";
    }

    out << "</table>"
           "</body>"
           "</html>";
    //****************************

    filename.close();
}
//-------------------------------------------------------

















