#include "rhttp.h"
//-------------------------------------------------------------
RCookie::RCookie(QObject *parent) :
    QNetworkCookieJar(parent)
{
}
//-------------------------------------------------------------
QList<QNetworkCookie> RCookie::getAllCookies(void)
{
    return this->allCookies();
}
//-------------------------------------------------------------
void RCookie::setAllSitesCookies(const QList<QNetworkCookie>& cookieList)
{
    this->setAllCookies(cookieList);
}
//-------------------------------------------------------------
RHttp::RHttp(QObject *parent) :
    QObject(parent)
{
    this->manager    = new QNetworkAccessManager(this);
    this->manager->setCookieJar(this->cookieBuff = new RCookie(this));

    this->m_autoRedirect = false;
    this->m_ajax = false;
    this->m_json = false;
    this->useProxy = false;
    this->m_userAgent.clear();
    this->m_referer.clear();
    this->m_timeOut = 60000;
    this->m_connection.clear();
    this->m_contentType.clear();
    this->m_Ansii = false;
    this->m_gzip = false;
}
//-------------------------------------------------------------
RHttp::~RHttp()
{
    this->manager->deleteLater();
}
//-------------------------------------------------------------
bool RHttp::get(const QString &uri)
{
    QString zzz;
    //Clear
    this->m_inbuff.clear();
    this->m_httpStatusCode =0;
    this->m_httpReasonPhrase.clear();
    this->m_Location302.clear();

    //Прокси
    if(this->useProxy) {
       QNetworkProxy proxy;
       proxy.setType(static_cast<QNetworkProxy::ProxyType>(this->prx_dat.proxyType));
       proxy.setHostName(this->prx_dat.host);
       proxy.setPort(this->prx_dat.port);
       if(!this->prx_dat.user.isEmpty()) {
           proxy.setUser(this->prx_dat.user);
           proxy.setPassword(this->prx_dat.password);
       }
       this->manager->setProxy(proxy);
    }

    QUrl url(uri);
    QNetworkRequest request(url);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
    if(this->m_connection.isEmpty())
        request.setRawHeader("Connection", "keep-alive");
    else{
        request.setRawHeader("Connection", this->m_connection.toUtf8());
        this->m_connection.clear();
    }
    if(!this->m_userAgent.isEmpty())
        request.setRawHeader("User-Agent", this->m_userAgent.toUtf8());
    else
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:29.0) Gecko/20100101 Firefox/29.0 SeaMonkey/2.26");

    //Referer
    if(!this->m_referer.isEmpty()) {
       request.setRawHeader("Referer", this->m_referer.toUtf8());
       this->m_referer.clear();
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;

    QNetworkReply *reply = this->manager->get(request);
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                     this, SLOT(slotSslErrors(QList<QSslError>)));

    timer.start(this->m_timeOut);   //timeout
    loop.exec(); //Waiting for request finished

    QObject::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    if(timer.isActive()) {
        timer.stop();
        //Save code of network connection
        this->m_networkError = reply->error();
        //Save http code: 200, 304, 404, 401, etc
        this->m_httpStatusCode   = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        this->m_httpReasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        //If conection error - return false
        if(reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::ContentNotFoundError) {
             reply->deleteLater();
             return false;
        }
        //Processing of Http Status Code, currently 302
        if((this->m_httpStatusCode == 302 || this->m_httpStatusCode == 301) && this->m_autoRedirect == true) {
            QUrl reUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            if(reUrl.isRelative()) reUrl = url.resolved(reUrl).toString();
            reply->deleteLater();
            zzz = reUrl.toString();
            return this->get(reUrl.toString());
        }
        else //Save request answer (inbuff)
        {
            if(this->m_httpStatusCode == 302 || this->m_httpStatusCode == 301)
                this->m_Location302 = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            reply->deleteLater();
            this->m_inbuff = reply->readAll();
            this->m_listHeaders = &reply->rawHeaderPairs();
            return true;
        }
    }

    //Timeout
    this->m_networkError = QNetworkReply::TimeoutError;
    reply->abort();
    reply->deleteLater();
    return false;
}
//-------------------------------------------------------------

void RHttp::slotSslErrors(QList<QSslError> err)
{
    int i=0;
    i++;
}
//-------------------------------------------------------------
bool RHttp::post(const QString &uri, const QByteArray &data)
{
    //Clear
    this->m_inbuff.clear();
    this->m_httpStatusCode =0;
    this->m_httpReasonPhrase.clear();
    this->m_Location302.clear();

    //Прокси
    if(this->useProxy) {
       QNetworkProxy proxy;
       proxy.setType(static_cast<QNetworkProxy::ProxyType>(this->prx_dat.proxyType));
       proxy.setHostName(this->prx_dat.host);
       proxy.setPort(this->prx_dat.port);
       if(!this->prx_dat.user.isEmpty()) {
           proxy.setUser(this->prx_dat.user);
           proxy.setPassword(this->prx_dat.password);
       }
       this->manager->setProxy(proxy);
    }

    QUrl url(uri);
    //QEventLoop eventLoop;
    QNetworkRequest request(url);
    if(!this->m_userAgent.isEmpty())
        request.setRawHeader("User-Agent", this->m_userAgent.toUtf8());
    else
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; rv:19.0) Gecko/20100101 Firefox/19.0");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
    if(this->m_gzip == true){
        this->m_gzip = false;
        request.setRawHeader("Accept-Encoding", "gzip, deflate");
    }
    request.setRawHeader("Cache-Control", "no-cache");
    if(this->m_contentType.isEmpty()){
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }else{
        request.setHeader(QNetworkRequest::ContentTypeHeader, this->m_contentType.toUtf8());
        this->m_contentType.clear();
    }
    if(this->m_json == true) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }
    //Is Ajax request
    if(this->m_ajax == true) {
        this->m_ajax = false;
        request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    }
    //Referer
    if(!this->m_referer.isEmpty()) {
       request.setRawHeader("Referer", this->m_referer.toUtf8());
       this->m_referer.clear();
    }
    if(this->m_connection.isEmpty()){
        request.setRawHeader("Connection", "keep-alive");
    }else{
        request.setRawHeader("Connection", this->m_connection.toUtf8());
        this->m_connection.clear();
    }
    request.setRawHeader("Pragma", "no-cache");

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;

    QNetworkReply *reply;
    if(this->m_json == true){
        reply = this->manager->post(request, data);
        this->m_json = false;
    }else{
        if(this->m_Ansii == false)
            reply = this->manager->post(request, data.toPercentEncoding("&="));
        else{
            this->m_Ansii = false;
            QTextCodec *wincodec = QTextCodec::codecForName("Windows-1251");
            QString text = data;
            reply = this->manager->post(request, wincodec->fromUnicode(text).toPercentEncoding("&="));
        }
    }
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(this->m_timeOut);   //timeout
    loop.exec(); //Waiting for request finished

    QObject::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    if(timer.isActive()) {
        timer.stop();
        //Save code of network connection
        this->m_networkError = reply->error();
        //Save http code: 200, 304, 404, 401, etc
        this->m_httpStatusCode   = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        this->m_httpReasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        //If conection error - return false
        if(reply->error() != QNetworkReply::NoError) {
             reply->deleteLater();
             return false;
        }
        //Processing of Http Status Code, currently 302
        if((this->m_httpStatusCode == 302 || this->m_httpStatusCode == 301) && this->m_autoRedirect == true) {
            QUrl reUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            if(reUrl.isRelative()) reUrl = url.resolved(reUrl).toString();
            reply->deleteLater();
            return this->get(reUrl.toString());
        }
        else //Save request answer (inbuff)
        {
            if(this->m_httpStatusCode == 302 || this->m_httpStatusCode == 301)
                this->m_Location302 = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            reply->deleteLater();
            this->m_inbuff = reply->readAll();
            this->m_listHeaders = &reply->rawHeaderPairs();
            return true;
        }
    }

    //Timeout
    this->m_networkError = QNetworkReply::TimeoutError;
    reply->abort();
    reply->deleteLater();
    return false;
}
//-------------------------------------------------------------
//Загрузка файла на сервер
//Принимает 3 аргумента:
//1) URL строка -  куда грузиться файл
//2) Структура типа uploadFileData, которая состоит из 3 полей:
//       a)typeData - Тип данных, например: image/jpeg
//       b)header - заголовок данных, например: form-data; name="Filedata[]"; filename="_n_qx8j179Q.jpg"
//       c)body - тело передаваемых данных в формате QByteArray
//3)Дополнительные поля для header
//  Например: Заголовок form-data; name="s_post"
//            Данные заголовка: 98309a5b5b1fbebd0d60a446a38534e9
bool RHttp::upload(const QString &uri, uploadFileData *data, QList< QPair<QString,QString> > *param)
{
    //Clear
    this->m_inbuff.clear();
    this->m_httpStatusCode =0;
    this->m_httpReasonPhrase.clear();
    this->m_Location302.clear();

    //Прокси
    if(this->useProxy) {
       QNetworkProxy proxy;
       proxy.setType(static_cast<QNetworkProxy::ProxyType>(this->prx_dat.proxyType));
       proxy.setHostName(this->prx_dat.host);
       proxy.setPort(this->prx_dat.port);
       if(!this->prx_dat.user.isEmpty()) {
           proxy.setUser(this->prx_dat.user);
           proxy.setPassword(this->prx_dat.password);
       }
       this->manager->setProxy(proxy);
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

   //Загружаем данные param
    QPair<QString,QString> itemParam;
    if(param != nullptr) {
        for(int i=0; i < param->size(); ++i) {
            QHttpPart dataPart;
            dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(param->at(i).first));
            dataPart.setBody(param->at(i).second.toUtf8());
            multiPart->append(dataPart);
        }
    }

    //Загружаем файл
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(data->typeData));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(data->header));
    filePart.setBody(data->body);
    multiPart->append(filePart);

    //Формируем заголовки запроса
    QUrl url(uri);
    QEventLoop eventLoop;

    QNetworkRequest request(url);
    if(!this->m_userAgent.isEmpty())
        request.setRawHeader("User-Agent", this->m_userAgent.toUtf8());
    else
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1; rv:19.0) Gecko/20100101 Firefox/19.0");

    request.setRawHeader("Accept", "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
    //Is Ajax request
    if(this->m_ajax == true) {
        this->m_ajax = false;
        request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    }
    //Referer
    if(!this->m_referer.isEmpty()) {
       request.setRawHeader("Referer", this->m_referer.toUtf8());
       this->m_referer.clear();
    }
    if(this->m_connection.isEmpty())
        request.setRawHeader("Connection", "keep-alive");
    else{
        request.setRawHeader("Connection", this->m_connection.toUtf8());
        this->m_connection.clear();
    }
    request.setRawHeader("Pragma", "no-cache");

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;

    QNetworkReply *reply = this->manager->post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(this->m_timeOut);   //timeout
    loop.exec(); //Waiting for request finished

    QObject::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    if(timer.isActive()) {
        timer.stop();
        //Save code of network connection
        this->m_networkError = reply->error();
        //Save http code: 200, 304, 404, 401, etc
        this->m_httpStatusCode   = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        this->m_httpReasonPhrase = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        //If conection error - return false
        if(reply->error() != QNetworkReply::NoError) {
             reply->deleteLater();
             return false;
        }
        //Processing of Http Status Code, currently 302
        if(this->m_httpStatusCode == 302 && this->m_autoRedirect == true) {
            QUrl reUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            if(reUrl.isRelative()) reUrl = url.resolved(reUrl).toString();
            reply->deleteLater();
            return this->get(reUrl.toString());
        }
        else //Save request answer (inbuff)
        {
            if(this->m_httpStatusCode == 302)
                this->m_Location302 = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
            reply->deleteLater();
            this->m_inbuff = reply->readAll();
            this->m_listHeaders = &reply->rawHeaderPairs();
            return true;
        }
    }

    //Timeout
    this->m_networkError = QNetworkReply::TimeoutError;
    reply->abort();
    reply->deleteLater();
    return false;
}
//-------------------------------------------------------------
//Отобразить Header полученого заголовка в виде текста
QString RHttp::headersString()
{
    QString headersString;
    QNetworkReply::RawHeaderPair header;

    headersString  = QString::number(this->m_httpStatusCode);
    headersString += " | ";
    headersString += this->m_httpReasonPhrase;
    headersString += "\r\n";

    int size = this->m_listHeaders->size();
    for(int i=0; i<size; ++i) {
        header = this->m_listHeaders->at(i);
        headersString += header.first;
        headersString += ": ";
        headersString += header.second;
        headersString += "\r\n";
    }
    return headersString;
}
//-------------------------------------------------------------
//Очистить cookie
void RHttp::clearCookie()
{
    this->manager->setCookieJar(this->cookieBuff = new RCookie(this));
}
//-------------------------------------------------------------
//Очистить прокси
void RHttp::clearProxy()
{
    this->useProxy = false;
}
//-------------------------------------------------------------
void RHttp::setProxy(int proxyType, QString host, int port, QString user, QString password)
{
    this->useProxy = true;
    this->prx_dat.proxyType = proxyType;
    this->prx_dat.host      = host;
    this->prx_dat.port      = static_cast<quint16>(port);
    this->prx_dat.user      = user;
    this->prx_dat.password  = password;
}
//-------------------------------------------------------------
void RHttp::saveInbuffToFile(QString dir)
{
    if(dir.isEmpty()) return;

    QFile file(dir);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << this->headersString();
    out << this->m_inbuff;
    file.close();
}
//-------------------------------------------------------------
QList<QNetworkCookie> RHttp::getAllCookies()
{
    return this->cookieBuff->getAllCookies();
}
//-------------------------------------------------------------
void RHttp::setAllCookies(const QList<QNetworkCookie>& cookies)
{
    this->cookieBuff->setAllSitesCookies(cookies);
}
//-------------------------------------------------------------






