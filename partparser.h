#ifndef PARTPARSER_H
#define PARTPARSER_H

#define AISTTOOLS   "aist-tools.ru"
#define GARAGETOOLS "garagetools.ru"
#define MACTAK      "mactak.ru"
#define AVTOALL     "avtoall.ru"
#define JTCRUSSIA   "jtcrussia.ru"

#define AUTOSCANERS  "autoscaners.ru"
#define CARMOD       "carmod.ru"
#define CARSCANNER   "carscanner.ru"
#define VGARAZHAX    "vgarazhax.ru"
#define CARHEALTH    "carhealth.ru"
#define ELECTRONICSSYSTEM "electronicsystem.ru"
#define MOSAUTOLAB   "mosautolab.ru"
#define AUTOSETDV    "autoset-dv.ru"
#define DIZEL        "di-zel.ru"
#define AUTOCHECKERS "autocheckers.ru"
#define XDIAG        "xdiag.ru"
#define DIAGNOSTICKS "diagnosticks.ru"

#define NO_RESULT "NO_RESULT"

#include <QObject>
#include <QStringList>
#include <QHash>
#include "rhttp.h"
#include "rstring.h"

enum parserStatus{
    PARSE_OK,
    PARSE_ERROR,
    PARSE_BADREQUEST
};

enum workerStatus{
    wsNOTHING,
    wsSUCCESS,
    wsERROR,
    wsBADREQUEST
};

enum wokerTask{
    wtNOTHING,
    wtPARSER
};

//Структура хранит настройки входных данных
struct in_option {
    QString field_separator; //Разделитель поля
    QString text_separator;  //Разделитель текста
    void clear() {
        this->field_separator.clear();
        this->text_separator.clear();
    }
};

//Структура хранит данные конкурента
struct competitor {
    QString url;
    QString manufacture;
    QString articule;
    float   price;
    bool    isHave;
    void clear() {
        this->url.clear();
        this->manufacture.clear();
        this->articule.clear();
        this->price  = 0;
        this->isHave = false;
    }
};

//Структура хранит данные нашего магазина
struct lineData {
    QString manufacture;
    QString articule;
    bool    useAutoScaners;
    float   price;
    QList <competitor*> comp; //Список наших данных и конкурентов по артикулу и наименованию
    void clear() {
        this->manufacture.clear();
        this->articule.clear();
        this->price =0;
        this->useAutoScaners = false;
        competitor *tmp;
        while(!this->comp.isEmpty()) {
            tmp = this->comp.first();
            tmp->clear();
            this->comp.removeFirst();
            delete tmp;
        };
    }
};

struct parse_data {
    QString dirFile;            //Путь к входному файлу
    in_option inopt;            //Настройки входных данных
    QStringList domains;        //Домены
    QList <lineData*> listData; //Структура хранит обрабатывамую информацию
    bool useAutoScanerParser;   //использовать Автосканер.Ru парсер
    void clear() {
        this->dirFile.clear();
        this->inopt.clear();
        this->domains.clear();
        lineData *tmp;
        this->useAutoScanerParser = false;
        while(!this->listData.isEmpty()) {
            tmp = this->listData.first();
            tmp->clear();
            this->listData.removeFirst();
            delete tmp;
        };
    }
};

struct TInfo {
    int     status;
    int     task;
    int     lastError;
    QString errorText;
    lineData *ld;
    void clear() {
        this->status = wsNOTHING;
        this->task   = wtNOTHING;
        this->lastError =0;
        this->errorText.clear();
    }
};

class partParser : public QObject
{
    Q_OBJECT
public:
    explicit partParser(QObject *parent = nullptr);
    int doParser(lineData *ld);

private:
    RHttp      *http;
    bool Get(const QString &url);
    bool Post(const QString &url, const QByteArray &data);
    QString currentParser;
    QStringList listUrl;
    QHash <QString, int (partParser::*)(competitor *)> pParseFunc;
    bool isHaveParseFunc(QString url);
    int  doParserCarTool(competitor *ld);     //парсинг для сайта www.car-tool.ru
    int  doParserAistTools(competitor *ld);   //Парсинг для сайта www.aist-tools.ru
    int  doParserGarageTools(competitor *ld); //Парсинг для сайта www.garagetools.ru
    int  doParserMactak(competitor *ld);      //Парсинг для сайта www.mactak.ru
    int  doParserAvtoal(competitor *ld);      //Парсинг для сайта www.avtoall.ru
    int  doParserJtcrussia(competitor *ld);   //Парсинг для сайта www.jtcrussia.ru

    int  doParserAutoscaners(competitor *ld);      //Парсинг для сайта autoscaners.ru
    int  doParserCarmod(competitor *ld);           //Парсинг для сайта carmod.ru
    int  doParserCarscanner(competitor *ld);       //Парсинг для сайта carscanner.ru
    int  doParserVgarazhax(competitor *ld);        //Парсинг для сайта vgarazhax.ru
    int  doParserCarhealth(competitor *ld);        //Парсинг для сайта carhealth.ru
    int  doParserElectronicsystem(competitor *ld); //Парсинг для сайта electronicsystem.ru
    int  doParserMosAutoLab(competitor *ld);       //Парсинг для сайта mosautolab.ru
    int  doParserAutoSetDv(competitor *ld);        //Парсинг для сайта autoset-dv.ru
    int  doParserDizel(competitor *ld);            //Парсинг для сайта di-zel.ru
    int  doParserAutocheckers(competitor *ld);     //Парсинг для сайта autocheckers.ru
    int  doParserXdiag(competitor *ld);            //Парсинг для сайта xdiag.ru
    int  doParserDiagnosticks(competitor *ld);     //Парсинг для сайта www.diagnosticks.ru

    void pauseSec(int sec);
    QString clearUrl(QString url);

signals:
    void addStatus(QString, QString, QString);
    void plusError();
    void plusBadRequest();

public slots:

protected:
    QString clearPrice(QString price);
    QString httpsToHttp(QString url);
    QString httpToHttps(QString url);
};

#endif // PARTPARSER_H
