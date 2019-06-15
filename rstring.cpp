#include "rstring.h"

//----------------------------------------------------------------------------------
bool RString::strstr(const RString &str, Qt::CaseSensitivity cs)
{
    return this->contains(str, cs);
}
//----------------------------------------------------------------------------------
RString RString::cut_to_str (const RString &src, Qt::CaseSensitivity cs)
{
    int find = this->indexOf(src, 0, cs);
    if(find < 0) return "";
    return this->left(find);
}
//----------------------------------------------------------------------------------
RString RString::cut_str_to_str (const RString &mstart, const RString &mend, Qt::CaseSensitivity cs)
{
    int findStart = this->indexOf(mstart, 0, cs);
    if(findStart < 0) return "";
    findStart = findStart + mstart.length();
    int findEnd = this->indexOf(mend, findStart, cs);
    if(findEnd < 0) return "";
    return this->mid(findStart, findEnd - findStart);
}
//----------------------------------------------------------------------------------
RString RString::erase_data (unsigned int count)
{
    if(count < 1) return "";
    RString deleted = this->left(count);
    this->remove(0, count);
    return deleted;
}
//----------------------------------------------------------------------------------
RString RString::erase_to (const RString &met, Qt::CaseSensitivity cs)
{
    int find = this->indexOf(met, 0, cs);
    if(find < 0) return "";
    RString deleted = this->left(find);
    this->remove(0, find);
    return deleted;
}
//----------------------------------------------------------------------------------
RString RString::erase_with (const RString &met, Qt::CaseSensitivity cs)
{
    int find = this->indexOf(met, 0, cs);
    if(find < 0) return "";
    find = find + met.length();
    RString deleted = this->left(find);
    this->remove(0, find);
    return deleted;
}
//----------------------------------------------------------------------------------
bool RString::saveToFile (const RString &fname)
{
    if(fname.isEmpty()) return false;
    QFile file(fname);
    if(!file.open(QIODevice::WriteOnly))
        return false;
    file.write(this->toUtf8(), this->length());
    file.close();
    return true;
}
//----------------------------------------------------------------------------------
bool RString::appendToFile (const RString &fname)
{
    if(fname.isEmpty()) return false;
    QFile file(fname);
    if(!file.open(QIODevice::Append))
        return false;
    file.write(this->toUtf8(), this->length());
    file.close();
    return true;
}
//----------------------------------------------------------------------------------
bool RString::readFromFile (const RString &fname)
{
    if(fname.isEmpty()) return false;
    QFile file(fname);
    if(!file.open(QIODevice::ReadOnly))
        return false;
    QByteArray tmp = file.readAll();
    file.close();
    QString myString(tmp);
    this->clear();
    this->append(myString);
    return true;
}
//----------------------------------------------------------------------------------
