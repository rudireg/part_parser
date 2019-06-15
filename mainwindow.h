#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include "master.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Master  *master;
    QThread *thread;
    parse_data *pd;
    QSettings m_settings;
    void writeSettings();
    void readSettings();

signals:
    void initMaster(int, parse_data*, bool);
    void stopWork();

public slots:
    void showBar(QString msg);
    void showMsgBox(QString msg);
    void showCounters(counters *cnt);
    void plusProgressBar();
    void clearProgressBar();
    void setMaxValueProgressBar(int val);
    void addStatusToTableWidget(int idThread, QString article, QString status, QString color);
    void enabledStart();


private slots:
    void on_pushButton_Stop_clicked();
    void on_pushButton_Start_clicked();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
