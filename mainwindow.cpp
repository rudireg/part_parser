#include "mainwindow.h"
#include "ui_mainwindow.h"

//--------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings("RudiSoft","PartParser")
{
    ui->setupUi(this);
    this->master = new Master;
    this->master->setObjectName("mainMaster");
    this->thread = new QThread;
    this->thread->setObjectName("threadMaster");

   //connect(this->thread, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(this->master, SIGNAL(showBar(QString)), this, SLOT(showBar(QString)));
    connect(this,SIGNAL(initMaster(int,parse_data*,bool)),this->master,SLOT(initMaster(int,parse_data*,bool)));
    connect(this->master,SIGNAL(showCounters(counters*)),this,SLOT(showCounters(counters*)));
    connect(this->master,SIGNAL(setMaxValueProgressBar(int)),this,SLOT(setMaxValueProgressBar(int)));
    connect(this->master,SIGNAL(plusProgressBar()),this,SLOT(plusProgressBar()));
    connect(this->master,SIGNAL(clearProgressBar()),this,SLOT(clearProgressBar()));
    connect(this->master,SIGNAL(addStatusToTableWidget(int,QString,QString,QString)),
            this,SLOT(addStatusToTableWidget(int,QString,QString,QString)));
    connect(this,SIGNAL(stopWork()),this->master,SLOT(stopWork()));
    connect(this->master,SIGNAL(enabledStart()),this,SLOT(enabledStart()));
    connect(this->master,SIGNAL(showMsgBox(QString)),this,SLOT(showMsgBox(QString)));

    this->master->moveToThread(this->thread);
    this->thread->start();

    this->pd = nullptr;
    //Инициализировать настройки
    this->readSettings();
}
//--------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    //Сохранить настройки
    this->writeSettings();

    delete ui;
    if(this->pd != nullptr) {
        this->pd->clear();
        delete this->pd;
    }
}
//--------------------------------------------------------------------------
void MainWindow::showBar(QString msg)
{
    ui->statusBar->showMessage(msg);
}
//--------------------------------------------------------------------------
void MainWindow::writeSettings()
{
    m_settings.beginGroup("/Settings");
    m_settings.setValue("/fieldSeparator", ui->field_separator->currentIndex());
    m_settings.setValue("/textSeparator",  ui->text_separator->currentIndex());
    m_settings.endGroup();
}
//--------------------------------------------------------------------------
void MainWindow::readSettings()
{
    m_settings.beginGroup("/Settings");
    //Read
    int fieldSeparator = m_settings.value("/fieldSeparator", 0).toInt();
    int textSeparator  = m_settings.value("/textSeparator", 0).toInt();
    //Set
    ui->field_separator->setCurrentIndex(fieldSeparator);
    ui->text_separator->setCurrentIndex(textSeparator);
    m_settings.endGroup();
}
//--------------------------------------------------------------------------
//Показать счетчики
void MainWindow::showCounters(counters *cnt)
{
    ui->cnt_all_article->setNum(cnt->totalLines);
    ui->cnt_done->setNum(cnt->compleatedLines);
    ui->cnt_error->setNum(cnt->errors);
    ui->cnt_bad_request->setNum(cnt->badRequest);
}
//--------------------------------------------------------------------------
//Прибавить прогресс бар
void MainWindow::plusProgressBar()
{
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}
//--------------------------------------------------------------------------
void MainWindow::clearProgressBar()
{
    ui->progressBar->setValue(0);
}
//--------------------------------------------------------------------------
//Установить максимальное значение для прогресс бар
void MainWindow::setMaxValueProgressBar(int val)
{
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(val);
}
//--------------------------------------------------------------------------
//Отображает текущее состояние потока
void MainWindow::addStatusToTableWidget(int idThread, QString article, QString status, QString color)
{
    ui->tableWidget->item(idThread,0)->setText(article);
    ui->tableWidget->item(idThread,1)->setText(status);

    QRadialGradient gradient(0, 50, 100, 50, 100);
    if(color == "red")         gradient.setColorAt(0, QColor::fromRgbF(1, 0, 0, 1));
    else if(color == "green")  gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    else if(color == "blue")   gradient.setColorAt(0, QColor::fromRgbF(0, 0, 1, 1));
    else if(color == "orange") gradient.setColorAt(0, QColor::fromRgbF(1, 1, 0, 1));
    else                       gradient.setColorAt(0, QColor::fromRgbF(0, 1, 1, 1));

    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    QBrush brush(gradient);
    ui->tableWidget->item(idThread, 1)->setBackground(brush);
}
//--------------------------------------------------------------------------
void MainWindow::enabledStart()
{
    ui->tableWidget->clear();
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Start->setEnabled(true);
   // QMessageBox::information(NULL,"Info","Parser is Done");
}
//--------------------------------------------------------------------------
void MainWindow::showMsgBox(QString msg)
{
    QMessageBox::information(nullptr,"Info",msg);
}
//--------------------------------------------------------------------------
void MainWindow::on_pushButton_Stop_clicked()
{
    ui->pushButton_Stop->setEnabled(false);
    emit stopWork();
}
//--------------------------------------------------------------------------
void MainWindow::on_pushButton_Start_clicked()
{
    int cntThr = ui->countThreads->text().toInt();  //Кол. потоков
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(cntThr);
    QTableWidgetItem *valuesHeaderItem = new QTableWidgetItem(QString::fromUtf8("Артикул"));
    ui->tableWidget->setHorizontalHeaderItem(0, valuesHeaderItem);
    valuesHeaderItem = new QTableWidgetItem(QString::fromUtf8("Статус"));
    ui->tableWidget->setHorizontalHeaderItem(1, valuesHeaderItem);
    ui->tableWidget->resizeColumnsToContents();

    QTableWidgetItem *newItem;
    for(int i=0; i<cntThr; ++i) {
        ui->tableWidget->setRowHeight(i,20);
        newItem = new QTableWidgetItem("-");
        ui->tableWidget->setItem(i,0,newItem);
        newItem = new QTableWidgetItem("-");
        ui->tableWidget->setItem(i,1,newItem);
    }

    bool useProxy = false; //Использовать прокси
    int countThreads = ui->countThreads->text().toInt(); //Кол. потоков

    //Указываем разделители строк и колонок, а так же назначаем имя обрабатыаемого файла
    if(this->pd != nullptr) {
        this->pd->clear();
        delete this->pd;
    }
    this->pd = new parse_data;

    this->pd->dirFile = ui->dirInFile->text();
    QString sep = ui->field_separator->currentText();
    if(sep.contains("Пробел")) sep = ' ';
    else if(sep.contains("Таб")) sep = '\t';
    this->pd->inopt.field_separator = sep;
    this->pd->inopt.text_separator  = ui->text_separator->currentText();
    this->pd->useAutoScanerParser = (ui->autoscanerParser->isChecked() == true)? true : false;

    emit initMaster(countThreads, this->pd, useProxy);
    ui->pushButton_Start->setEnabled(false);
    ui->pushButton_Stop->setEnabled(true);
}
//--------------------------------------------------------------------------
void MainWindow::on_pushButton_clicked()
{
    QString tmp = QDir::currentPath();
    QString str = QFileDialog::getOpenFileName(this,"Select File",QDir::current().absolutePath(), "*.csv *.txt");
    if(!str.isEmpty()) {
        ui->dirInFile->setText(str);
        ui->pushButton_Start->setEnabled(true);
    }
    QDir::setCurrent(tmp);
}
//--------------------------------------------------------------------------












