#include "partparser.h"
//-------------------------------------------------------
partParser::partParser(QObject *parent) :
    QObject(parent)
{
    //HTTP протокол
    this->http = new RHttp(this);
    this->http->setTimeOut(30000);

    //Регистрация доступных сайтов для парсинга
    this->listUrl.append(QString(GARAGETOOLS));
    this->listUrl.append(QString(AISTTOOLS));
    this->listUrl.append(QString(MACTAK));
    this->listUrl.append(QString(AVTOALL));
    this->listUrl.append(QString(JTCRUSSIA));

    this->listUrl.append(QString(AUTOSCANERS));
    this->listUrl.append(QString(CARMOD));
    this->listUrl.append(QString(CARSCANNER));
    this->listUrl.append(QString(VGARAZHAX));
    this->listUrl.append(QString(CARHEALTH));
    this->listUrl.append(QString(ELECTRONICSSYSTEM));
    this->listUrl.append(QString(MOSAUTOLAB));
    this->listUrl.append(QString(AUTOSETDV));
    this->listUrl.append(QString(DIZEL));
    this->listUrl.append(QString(AUTOCHECKERS));
    this->listUrl.append(QString(XDIAG));
    this->listUrl.append(QString(DIAGNOSTICKS));

    this->pParseFunc[GARAGETOOLS] = &partParser::doParserGarageTools;
    this->pParseFunc[AISTTOOLS]   = &partParser::doParserAistTools;
    this->pParseFunc[MACTAK]      = &partParser::doParserMactak;
    this->pParseFunc[AVTOALL]     = &partParser::doParserAvtoal;
    this->pParseFunc[JTCRUSSIA]   = &partParser::doParserJtcrussia;

    this->pParseFunc[AUTOSCANERS]       = &partParser::doParserAutoscaners;
    this->pParseFunc[CARMOD]            = &partParser::doParserCarmod;
    this->pParseFunc[CARSCANNER]        = &partParser::doParserCarscanner;
    this->pParseFunc[VGARAZHAX]         = &partParser::doParserVgarazhax;
    this->pParseFunc[CARHEALTH]         = &partParser::doParserCarhealth;
    this->pParseFunc[ELECTRONICSSYSTEM] = &partParser::doParserElectronicsystem;
    this->pParseFunc[MOSAUTOLAB]        = &partParser::doParserMosAutoLab;
    this->pParseFunc[AUTOSETDV]         = &partParser::doParserAutoSetDv;
    this->pParseFunc[DIZEL]             = &partParser::doParserDizel;
    this->pParseFunc[AUTOCHECKERS]      = &partParser::doParserAutocheckers;
    this->pParseFunc[XDIAG]             = &partParser::doParserXdiag;
    this->pParseFunc[DIAGNOSTICKS]      = &partParser::doParserDiagnosticks;
}
//-------------------------------------------------------
bool partParser::Get(const QString &url)
{
    int step =0, limit =3;
    bool rez;
    do{
        rez = this->http->get(url);
        step ++;
        if(step > 1) this->pauseSec(5);
        if(step >= limit) break;
    }while(!rez);
    return rez;
}
//---------------------------------------------------------
bool partParser::Post(const QString &url, const QByteArray &data)
{
    int step =0, limit =3;
    bool rez;
    do{
        rez = this->http->post(url, data);
        step ++;
        if(step > 1) this->pauseSec(5);
        if(step >= limit) break;
    }while(!rez);
    return rez;
}
//-------------------------------------------------------
//Очистка URL от http://www.
QString partParser::clearUrl(QString url)
{
    QString head("http://www.");
    QString head2("http://");
    QString head3("https://www.");
    QString head4("https://");
    RString tmp = url;
    if(tmp.contains(head))
        tmp.erase_data(static_cast<unsigned>(head.size()));
    if(tmp.contains(head2))
        tmp.erase_data(static_cast<unsigned>(head2.size()));
    if(tmp.contains(head3))
        tmp.erase_data(static_cast<unsigned>(head3.size()));
    if(tmp.contains(head4))
        tmp.erase_data(static_cast<unsigned>(head4.size()));
    if(tmp.contains("/"))
        tmp = tmp.erase_to("/");
    head = tmp;
    return head;
}
//-------------------------------------------------------
// Чистим цену от мусора
QString partParser::clearPrice(QString price)
{
    // если содержит и точку и запятую
    if (price.contains(".") && price.contains(",")) {
        price = price.remove(price.indexOf("."), price.size());
        return price.remove(QRegExp("[^0-9]"));
    } else if (price.contains(".")) {
        price = price.remove(price.indexOf("."), price.size());
        return price.remove(QRegExp("[^0-9]"));
    } else if (price.contains(",")) {
        return price.remove(QRegExp("[^0-9]"));
    }
    return price.remove(QRegExp("[^0-9]"));
}
//-------------------------------------------------------
// Замена https:// на http://
QString partParser::httpsToHttp(QString url)
{
    if (url.contains("https://")) {
        url.replace(0, 5, "http");
    }
    return url;
}
//-------------------------------------------------------
// Замена http:// на https://
QString partParser::httpToHttps(QString url)
{
    if (url.contains("http://")) {
        url.replace(0, 4, "https");
    }
    return url;
}
//-------------------------------------------------------
//Произвести парсинг
int partParser::doParser(lineData *ld)
{
    if(ld->useAutoScaners == true){
        //Узнаем цены на товары у конкурентов
        foreach(competitor *cmp, ld->comp) {
            if(cmp->isHave == false)
                continue;
            //Определяем - есть ли функция по обработке текущего URL
            //Назначем функции указателю парсеру - нужный вариант парсинга
            cmp->url = this->clearUrl(cmp->url);
            if(!this->isHaveParseFunc(cmp->url))
                continue;
            //Делаем парсинг на стороних сайтах
            //emit addStatus(ld->articule, QString("Парсим %1").arg(this->currentParser), "green");
            emit addStatus(ld->articule, QString("Парсим %1").arg(cmp->articule), "green");
            int rez = (this->*pParseFunc[this->currentParser])(cmp);
            if(rez == PARSE_OK)
                emit addStatus(ld->articule, QString("Отработали %1").arg(this->currentParser), "green");
            else if(rez == PARSE_ERROR) {
                emit addStatus(ld->articule, QString("Ошибка %1").arg(this->currentParser), "red");
                //Увеличиваем счетчик ошибок
                emit plusError();
                //Пауза
                this->pauseSec(1);
            }
            else {
                cmp->price = -1;
                emit addStatus(ld->articule, QString("Bad Request %1").arg(this->currentParser), "orange");
                //Увеличиваем счетчик ошибок
                emit plusBadRequest();
                //Пауза
                this->pauseSec(1);
            }
        }
    }else{
        //Узнаем цену на www.car-tool.ru
        emit addStatus(ld->articule, QString("Парсим www.car-tool.ru артикул %1").arg(ld->articule), "green");
        competitor *ownSite = new competitor;
        ownSite->clear();
        ownSite->url = "http://www.car-tool.ru";
        ownSite->articule = ld->articule;
        this->doParserCarTool(ownSite);
        ld->price = ownSite->price;

        //Узнаем цены на товары у конкурентов
        foreach(competitor *cmp, ld->comp) {
            if(cmp->isHave == false)
                continue;
            //Определяем - есть ли функция по обработке текущего URL
            //Назначем функции указателю парсеру - нужный вариант парсинга
            if(!this->isHaveParseFunc(cmp->url))
                continue;
            //Делаем парсинг на стороних сайтах
            emit addStatus(ld->articule, QString("Парсим %1").arg(this->currentParser), "green");

            int rez = (this->*pParseFunc[this->currentParser])(cmp);
            if(rez == PARSE_OK)
                emit addStatus(ld->articule, QString("Отработали %1").arg(this->currentParser), "green");
            else if(rez == PARSE_ERROR) {
                emit addStatus(ld->articule, QString("Ошибка %1").arg(this->currentParser), "red");
                //Увеличиваем счетчик ошибок
                emit plusError();
                //Пауза
                this->pauseSec(1);
            }
            else {
                cmp->price = -1;
                emit addStatus(ld->articule, QString("Bad Request %1").arg(this->currentParser), "orange");
                //Увеличиваем счетчик ошибок
                emit plusBadRequest();
                //Пауза
                this->pauseSec(1);
            }
        }
    }
    return PARSE_OK;
}
//-------------------------------------------------------
//Проверить есть ли парсинг для данного сайта ?
//Если есть, то назначить функции указателю - указател на функцию парсинга
bool partParser::isHaveParseFunc(QString url)
{
    if(url.isEmpty()) return false;
    if(url[url.length()-1] == '/') url.chop(1); //Удаляем в конце слэш /
    //Перебираем URL в поисках нужного
    foreach(const QString &lst, this->listUrl) {
        if(url.contains(lst)) {
            this->currentParser = lst;
            return true;
        }
    };
    return false;
}
//-------------------------------------------------------
//Парсинг для сайта www.car-tool.ru
int partParser::doParserCarTool(competitor *ld)
{
    RString inb, block, tmp, name;
    QString start("<div class=\"catalog\">"); //<div class="catalog">
    QString end("<div class=\"basket inor"); //<div class="basket inor
    QString metItem("<div class=\"item\">"); //<div class="item">
    QString data("http://www.car-tool.ru/search/index.php?q=");
    QString skidka("class=\"right\">"); //class="right">
    data += ld->articule;
    this->http->clearCookie();
    this->http->setReferer("http://www.car-tool.ru/");
    if(!this->Get(data))
        return PARSE_BADREQUEST;
    if(this->http->inbuff().contains("<p>The document has moved <a href=")){
        inb = this->http->inbuffQString();
        tmp = inb.cut_str_to_str("<p>The document has moved <a href=\"", "\"");
        if(!this->Get(tmp))
            return PARSE_BADREQUEST;
    }
    //this->http->saveInbuffToFile("ERROR/carTool.html");
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(start)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    inb.erase_to(start);
    while(inb.contains(metItem))
    {
        if(!inb.contains(metItem) || !inb.contains(end))
            return PARSE_ERROR;
        block = inb.cut_str_to_str(metItem, end);
        inb.erase_to(metItem);
        inb.erase_data(1);
        name = QString(">%1<").arg(ld->articule);
        if(block.contains(name)){
            //Парсим цену
            block.erase_to("<div class=\"price");
            if(block.contains(skidka)){ //если есть скидка
                tmp = block.cut_str_to_str(skidka, "</span>");
                if(tmp.contains("<span>"))
                    tmp.erase_data(6);
            }else{
                tmp = block.cut_str_to_str("class=\"left\">", "</span>");
                if(tmp.contains("<span>"))
                    tmp.erase_data(6);
            }
            break;
        }
    };
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта garagetools.ru
int partParser::doParserGarageTools(competitor *ld)
{
    QString metNotFound("<p>Ничего не найдено,");
    RString inb, block, tmp, name, tmpBlock;
    QString data("http://garagetools.ru/search?utf8=%E2%9C%93&q=");
    data += ld->articule;
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://garagetools.ru/");
    if(!this->Get(data))
        return PARSE_BADREQUEST;
    inb = this->http->inbuffQString();
    //inb.saveToFile("ERROR/garagetools.html");

    //Если есть переадресация
    if(inb.contains(">redirected<")){
        data = inb.cut_str_to_str("<a href=\"", "\">");
        ld->manufacture = data;
        if(!this->Get(data))
            return PARSE_BADREQUEST;
        inb = this->http->inbuffQString();
      //  inb.saveToFile("ERROR/garagetools2.html");
    }
    //если нет товара в каталоге
    if(inb.contains(metNotFound)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    if(inb.contains("class=\"product-price m-big")){ //Если в ответе 1 товар
        tmp = inb.cut_str_to_str("class=\"product-price m-big\">", "<");
        if(tmp.isEmpty())
            tmp = inb.cut_str_to_str("class=\"promo-price\">", "<");
    }else{ //Если в ответе много товаров
        //Перебираем результат поиска
        while(inb.contains("<tr>")) {
            block = inb.cut_str_to_str("<tr>", "</tr>");
            inb.erase_to("</tr>");
            inb.erase_data(1);
            name = ld->articule + "</a>";
            tmpBlock = block.cut_str_to_str("<td class=\"search_manufacturer\">", "</td>");
            if(tmpBlock.contains(name)){
                ld->manufacture = QString("http://garagetools.ru%1").arg(block.cut_str_to_str("href=\"", "\""));
                tmp = block.cut_str_to_str("search_price\">", "<");
                if(tmp.isEmpty())
                    tmp = block.cut_str_to_str("class=\"promo-price\">", "<");
                break;
            }
        };
    }
    if(ld->manufacture.isEmpty())
        return PARSE_ERROR;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта www.aist-tools.ru
int partParser::doParserAistTools(competitor *ld)
{
    RString inb, tmp, block, name;
    QString metNotFound("Сожалеем, но ничего не найдено");
    QString data("http://www.aist-tools.ru/catalog/?q=");
    data += ld->articule;
    data += "&s.x=56&s.y=26";
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.aist-tools.ru/");
    if(!this->Get(data))
        return PARSE_BADREQUEST;
    inb = this->http->inbuffQString();
    //inb.saveToFile("ERROR/aist-tools.html");
    //если нет товара в каталоге
    if(inb.contains(metNotFound)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    inb.erase_to("class=\"search-page\"");
    //Перебираем результат поиска
    while(inb.contains("<tr>")) {
        block = inb.cut_str_to_str("<tr>", "tlistitem_horizontal_shadow");
        inb.erase_to("tlistitem_horizontal_shadow");
        inb.erase_data(1);
        name = QString(">%1<").arg(ld->articule);
        if(block.contains(name)){
            ld->manufacture = QString("http://www.aist-tools.ru%1").arg(block.cut_str_to_str("href=\"", "\""));
            tmp = block.cut_str_to_str("<span class=\"price\">", " руб.<");
            break;
        }
    };
    if(ld->manufacture.isEmpty())
        return PARSE_ERROR;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта www.mactak.ru
int partParser::doParserMactak(competitor *ld)
{
    QString data = QString("name=%1&catid=0&displaycount=10&searchtype=exact").arg(ld->articule);
    RString inb, block, tmp, name;
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.mactak.ru/");
    this->http->setAjax(true);
    if(!this->Post("http://www.mactak.ru/store/ajaxsearch?ajax=1", data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    this->http->saveInbuffToFile("ERROR/mactak.ru.html");
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains("<tr ")) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    while(inb.contains("<tr ")) {
        block = inb.cut_str_to_str("<tr ", "</tr>");
        inb.erase_to("</tr>");
        inb.erase_data(1);
        name = QString("%1<").arg(ld->articule).toUtf8();
        if(block.contains(name)){
            ld->manufacture = QString("http://www.mactak.ru%1").arg(block.cut_str_to_str("href=\"", "\""));
            break;
        }
    };
    if(ld->manufacture.isEmpty())
        return PARSE_ERROR;
    this->http->get(ld->manufacture);
    inb = this->http->inbuffQString();
    //this->http->saveInbuffToFile("ERROR/mactak.ru_2.html");
    tmp = inb.cut_str_to_str("Цена: ", " руб.");
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта jtcrussia.ru
int partParser::doParserJtcrussia(competitor *ld)
{
    QString noFound(">Товары не найдены<");
    QString metStart("<h1>Поиск</h1>");
    QString metStartBlock("<article class=");
    QString metEndBlock("</article>");
    QString metPrice("div class=\"detail-price\">");
    QString data("http://www.jtcrussia.ru/search/?searchString=");
    data += ld->articule;
    RString inb, tmp, block, name;
    //отправляем запрос
    this->http->clearCookie();
    this->http->setAutoRedirect(false);
    this->http->setReferer("http://www.jtcrussia.ru/");
    if(!this->Get(data)) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    this->http->saveInbuffToFile("ERROR/jtcrussia.html");
    //если нет товара в каталоге
    if(inb.contains(noFound)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    if(!inb.contains(metStart)){
        return PARSE_ERROR;
    }
    inb.erase_to(metStart);
    while(inb.contains(metStartBlock)){
       block = inb.cut_str_to_str(metStartBlock, metEndBlock);
       inb.erase_to(metStartBlock);
       inb.erase_data(1);
       name = QString(">%1<").arg(ld->articule);
       if(block.contains(name)){
           ld->manufacture = QString("http://www.jtcrussia.ru%1").arg(block.cut_str_to_str("href=\"", "\""));
           if(!this->Get(ld->manufacture)) {
               return PARSE_BADREQUEST;
           }
           inb = this->http->inbuffQString();
           if(!inb.contains(metPrice)){
               return PARSE_ERROR;
           }
           inb.erase_to(metPrice);
           tmp = inb.cut_str_to_str("<u>", "<");
           break;
       }
    };
    if(ld->manufacture.isEmpty())
        return PARSE_ERROR;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта www.avtoall.ru
int partParser::doParserAvtoal(competitor *ld)
{
    QString noFound("мы не смогли ничего найти<");
    QString metEnd(">Найдены аналоги:<");
    QString metBlock("<div class=\"item\">");
    QString data("http://www.avtoall.ru/search/?GlobalFilterForm[namearticle]=");
    data += ld->articule;
    RString inb, tmp, block, name;
    //отправляем запрос
    this->http->clearCookie();
    this->http->setAutoRedirect(false);
    this->http->setReferer("http://www.avtoall.ru/");
    if(!this->Get(data)) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //this->http->saveInbuffToFile("ERROR/avtoall.html");
    //если нет товара в каталоге
    if(inb.contains(noFound)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    if(inb.contains(metEnd))
        inb = inb.erase_to(metEnd);
    while(inb.contains(metBlock)) {
        block = inb.cut_str_to_str(metBlock, "<div class=\"line\">");
        inb.erase_to(metBlock);
        inb.erase_data(1);
        name = QString(">Артикул: %1<").arg(ld->articule);
        if(block.contains(name)){
            ld->manufacture = QString("http://www.avtoall.ru%1").arg(block.cut_str_to_str("href=\"", "\""));
            tmp = block.cut_str_to_str("\"price-internet\">", " р");
            break;
        }
    };
    if(ld->manufacture.isEmpty())
        return PARSE_ERROR;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Пауза
void partParser::pauseSec(int sec)
{
    QEventLoop EL;
    QTimer timer;
    timer.singleShot((sec * 1000),&EL,SLOT(quit()));
    EL.exec();
}
//-------------------------------------------------------
//Парсинг для сайта autoscaners.ru
int partParser::doParserAutoscaners(competitor *ld)
{
    RString data, inb, tmp;
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("https://www.autoscaners.ru/");
    data = ld->articule;
    this->http->setAutoRedirect(true);

    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains("id=\"new-price\">")) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    ld->manufacture = ld->articule;
    tmp = inb.cut_str_to_str("id=\"new-price\">", "<");
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта carmod.ru
int partParser::doParserCarmod(competitor *ld)
{
    RString data, inb, tmp;
    QString endMet(">С ЭТИМ ТОВАРОМ ТАКЖЕ ПОКУПАЮТ<");
    QString met("class=\"js-product-price\">");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.carmod.ru");
    data = ld->articule;
    ld->articule = data = this->httpToHttps(data);
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    if(inb.contains(endMet))
        inb = inb.erase_to(endMet);
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    ld->manufacture = ld->articule;
    tmp = inb.cut_str_to_str(met, "</span>");
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта carscanner.ru
int partParser::doParserCarscanner(competitor *ld)
{
    RString data, inb, tmp, block;
    QString met("price product-page-price");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://carscanner.ru");
    data = ld->articule;
    this->http->setAutoRedirect(true);
    ld->articule = data = this->httpsToHttp(data);
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    block = inb.cut_str_to_str(met, "</p>");
    if (block.contains("<ins>")) {
        block = block.cut_str_to_str("<ins>", "</ins>");
    }
    ld->manufacture = ld->articule;
    tmp = block.cut_str_to_str("amount\">", "<span");
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта vgarazhax.ru   
int partParser::doParserVgarazhax(competitor *ld)
{
    RString data, inb, tmp;
    QString met("itemprop=\"price\" content=\"");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("https://vgarazhax.ru/");
    data = ld->articule;
    ld->articule = data = this->httpToHttps(data);
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    ld->manufacture = ld->articule;
    tmp = inb.cut_str_to_str(met, "\"");
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта http://www.carhealth.ru
int partParser::doParserCarhealth(competitor *ld)
{
    RString data, inb, tmp;
    QString met("<span class=\"PricesalesPrice\">");
    QString met2("<meta itemprop=\"price\" content=\"");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://carhealth.ru/");
    data = ld->articule;
    ld->articule = data = this->httpsToHttp(data);
   // data = QUrl::fromPercentEncoding(data.toUtf8());
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    if (!inb.contains(met) && !inb.contains(met2)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    if (inb.contains(met)) {
        tmp = inb.cut_str_to_str(met, "</span>");
    } else {
        tmp = inb.cut_str_to_str(met2, "\"");
    }
    ld->manufacture = ld->articule;
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта electronicsystem.ru
int partParser::doParserElectronicsystem(competitor *ld)
{
    RString data, inb, tmp, block;
    QString met("<div class=\"product-info"); //<div class="product-info
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://electronicsystem.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    inb.erase_to(met);
    block = inb.cut_str_to_str("<div class=\"price\">", "</div>");
    if(block.contains("class=\"price-new\">"))
        tmp = inb.cut_str_to_str("class=\"price-new\">", "р.");
    else
        tmp = inb.cut_str_to_str("<span>Цена:</span>", "р.");
    ld->manufacture = ld->articule;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта https://www.mosautolab.ru/
int partParser::doParserMosAutoLab(competitor *ld)
{
    RString data, inb, tmp;
    QString met("class=\"product-card-price__current\">"); //<span class="product-card-price__current">198 200 р.</span>
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("https://www.mosautolab.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    tmp = inb.cut_str_to_str(met, "р.<");
    ld->manufacture = ld->articule;
    //Если в сумме есть запятая
    while(tmp.contains(",")) {
        int pos = tmp.indexOf(",");
        tmp.remove(pos, tmp.size());
    }
    //Если в сумме есть точка
    while(tmp.contains(".")) {
        int pos = tmp.indexOf(".");
        tmp.remove(pos, tmp.size());
    }
    //Убираем из цены все символы кроме цифр
    tmp.remove(QRegExp("[^0-9]"));
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта http://www.autoset-dv.ru/
int partParser::doParserAutoSetDv(competitor *ld)
{
    RString data, inb, block, tmp;
    QString met("autoset-detail-info");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.autoset-dv.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met) || !inb.contains("data-price=\"")) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    block = inb.cut_str_to_str(met, "</div>");
    tmp = block.cut_str_to_str("data-price=\"", "\"");
    ld->manufacture = ld->articule;
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта http://di-zel.ru/
int partParser::doParserDizel(competitor *ld)
{
    RString data, inb, tmp;
    QString met("itemprop=\"price\" content=\"");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.autoset-dv.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    tmp = inb.cut_str_to_str(met, "\"");
    ld->manufacture = ld->articule;
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта https://autocheckers.ru/
int partParser::doParserAutocheckers(competitor *ld)
{
    RString data, inb, block, tmp;
    QString met("itemprop=\"offers\"");
    QString met2("autocalc-product-price");
    QString met3("'autocalc-product-special'");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://www.autoset-dv.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met) || !inb.contains(met2)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    block = inb.cut_str_to_str(met, "</div>");
    if (block.contains(met3)) { // скидка если есть
        tmp = block.cut_str_to_str(met3, "<");
    } else {
        tmp = block.cut_str_to_str(met2, "<");
    }
    ld->manufacture = ld->articule;
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта http://xdiag.ru/
int partParser::doParserXdiag(competitor *ld)
{
    RString data, inb, block, tmp;
    QString met("<div class=\"around_catalog_detail-price\">");
    QString met2("class=\"new_price\">");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("http://xdiag.ru/");
    data = ld->articule;
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met) || !inb.contains(met2)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    block = inb.cut_str_to_str(met, "</div>");
    tmp = block.cut_str_to_str(met2, "<");
    ld->manufacture = ld->articule;
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------
//Парсинг для сайта https://www.diagnosticks.ru/
int partParser::doParserDiagnosticks(competitor *ld)
{
    RString data, inb, tmp;
    QString met("data-hook=\"formatted-primary-price\">руб.");
    //отправляем запрос
    this->http->clearCookie();
    this->http->setReferer("https://www.diagnosticks.ru/");
    data = ld->articule;
    ld->articule = data = this->httpToHttps(data);
    if(!this->Get(data.toUtf8())) {
        return PARSE_BADREQUEST;
    }
    inb = this->http->inbuffQString();
    //если нет товара в каталоге
    if(!inb.contains(met)) {
        ld->manufacture = NO_RESULT;
        ld->price       = 0;
        return PARSE_OK;
    }
    tmp = inb.cut_str_to_str(met, "<");
    ld->manufacture = ld->articule;
    if (tmp.contains(",")) {
        tmp = tmp.remove(tmp.indexOf(","), tmp.size());
    }
    tmp = this->clearPrice(tmp);
    ld->price = tmp.toFloat();
    return PARSE_OK;
}
//-------------------------------------------------------







