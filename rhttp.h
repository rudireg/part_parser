#ifndef RHTTP_H
#define RHTTP_H

#define NO_POXY      2
#define SOCKS5_PROXY 1
#define HTTP_PROXY   3

#include <QObject>
#include <QByteArray>
#include <QSsl>
#include <QSslSocket>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QEventLoop>
#include <QUrl>
#include <QPair>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QNetworkCookie>

struct uploadFileData {
    QString typeData;
    QString header;
    QByteArray body;
};

struct proxy_data {
    QString host;
    quint16 port;
    int     proxyType;
    QString user;
    QString password;

    void clear() {
        this->host.clear();
        this->port =0;
        this->proxyType = NO_POXY;
        this->user.clear();
        this->password.clear();
    }
};

class RCookie : public QNetworkCookieJar
{
public:
    explicit RCookie(QObject *parent = nullptr);
    QList<QNetworkCookie> getAllCookies(void);
    void                  setAllSitesCookies(const QList<QNetworkCookie>& cookieList);
private:

};

class RHttp : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray m_inbuff READ inbuff)
    Q_PROPERTY(int m_networkError READ networkError)
    Q_PROPERTY(int m_httpStatusCode READ httpStatusCode)
    Q_PROPERTY(QString m_httpReasonPhrase READ httpReasonPhrase)
    Q_PROPERTY(bool m_autoRedirect READ autoRedirect WRITE setAutoRedirect)
    Q_PROPERTY(bool m_ajax READ ajax WRITE setAjax)
    Q_PROPERTY(bool m_json READ json WRITE setJson)
    Q_PROPERTY(QString m_userAgent READ userAgent WRITE setUserAgent)
    Q_PROPERTY(QString m_referer READ referer WRITE setReferer)
    Q_PROPERTY(const QList <QNetworkReply::RawHeaderPair> *m_listHeaders READ headersPair)
    Q_PROPERTY(int m_timeOut READ timeOut WRITE setTimeOut)
    Q_PROPERTY(QString m_Location302 READ Location302)
    Q_PROPERTY(QString m_connection READ getConnection WRITE setConnection)
    Q_PROPERTY(QString m_contentType READ getContentType WRITE setContentType)
    Q_PROPERTY(bool m_Ansii READ ansii WRITE setAnsii)
    Q_PROPERTY(bool m_gzip READ gzip WRITE setGzip)

public:
    explicit RHttp(QObject *parent = nullptr);
    ~RHttp();

    bool    get(const QString &uri);
    bool    post(const QString &uri, const QByteArray &data);
    bool    upload(const QString &uri, uploadFileData *data, QList< QPair<QString,QString> > *param);

    void    saveInbuffToFile(QString dir);
    void    clearCookie();
    QList<QNetworkCookie> getAllCookies();
    void    setAllCookies(const QList<QNetworkCookie>& cookies);
    void    clearProxy();
    void    setProxy(int proxyType, QString host, int port, QString user ="", QString password ="");

    //Property functions
    QByteArray inbuff() {return this->m_inbuff;}
    QString    inbuffQString() {return QString(this->m_inbuff);}
    int        networkError() {return this->m_networkError;}
    int        httpStatusCode() {return this->m_httpStatusCode;}
    QString    httpReasonPhrase() {return this->m_httpReasonPhrase;}
    bool       autoRedirect() {return this->m_autoRedirect;}
    void       setAutoRedirect(bool val) {this->m_autoRedirect = val;}
    bool       ajax() {return this->m_ajax;}
    void       setAjax(bool val) {this->m_ajax = val;}
    bool       json() {return this->m_json;}
    void       setJson(bool val) {this->m_json = val;}
    QString    userAgent() {return this->m_userAgent;}
    void       setUserAgent(QString ua) {this->m_userAgent = ua;}
    QString    referer(){return this->m_referer;}
    void       setReferer(QString ref) {this->m_referer = ref;}
    const QList <QNetworkReply::RawHeaderPair> *headersPair() {return this->m_listHeaders;}
    QString    headersString();
    int        timeOut() {return this->m_timeOut;}
    void       setTimeOut(int val) {this->m_timeOut = val;}
    QString    Location302() {return this->m_Location302;}
    QString    getConnection() {return this->m_connection;}
    void       setConnection(QString val) {this->m_connection = val;}
    QString    getContentType() {return this->m_contentType;}
    void       setContentType(QString val) {this->m_contentType = val;}
    bool       ansii() {return this->m_Ansii;}
    void       setAnsii(bool val) {this->m_Ansii = val;}
    bool       gzip() {return this->m_gzip;}
    void       setGzip(bool val) {this->m_gzip = val;}

private:
    QNetworkAccessManager *manager;
    bool useProxy;
    proxy_data prx_dat;
    RCookie *cookieBuff;

    const QList <QNetworkReply::RawHeaderPair> *m_listHeaders;
    int        m_timeOut;
    QByteArray m_inbuff;
    int        m_networkError;
    int        m_httpStatusCode;
    QString    m_httpReasonPhrase;
    bool       m_autoRedirect;
    QString    m_Location302;
    bool       m_ajax;
    bool       m_json;
    QString    m_userAgent;
    QString    m_referer;
    QString    m_connection;
    QString    m_contentType;
    bool       m_Ansii;
    bool       m_gzip;

signals:

public slots:

    void slotSslErrors(QList<QSslError> err);
};

#endif // RHTTP_H
