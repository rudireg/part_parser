#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codecUtf8 = QTextCodec::codecForName("utf8"); //Windows-1251
    QTextCodec::setCodecForLocale(codecUtf8);

    qRegisterMetaType<TInfo>("TInfo");
    qRegisterMetaType<parse_data>("parse_data");
    qRegisterMetaType<counters>("counters");

    MainWindow w;
    w.show();

    return a.exec();
}
