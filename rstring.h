#ifndef RSTRING_H
#define RSTRING_H

#include <QString>
#include <QFile>

class RString : public QString
{
public:
    RString ()                      : QString(){}
    RString (const char *str)       : QString(str){}
    RString (const QString &src)    : QString(src){}
    RString (char *str)             : QString(str){}
    RString (const QByteArray & ba) : QString(ba){}
    RString (const int &num);

    bool strstr(const RString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    RString cut_to_str(const RString &src, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    RString cut_str_to_str    (const RString &mstart, const RString &mend, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    RString erase_data (unsigned int count);
    RString erase_to   (const RString &met, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    RString erase_with (const RString &met, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    bool saveToFile   (const RString &fname);
    bool appendToFile (const RString &fname);
    bool readFromFile (const RString &fname);
};

#endif // RSTRING_H
