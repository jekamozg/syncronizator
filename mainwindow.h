#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include "webdav.h"

#include <QDebug>

#include "sqlite.h"

#include <QDir>

#include <QFileSystemModel>
#include <QFileSystemWatcher>

#include <QDateTime>

#include <QDataStream>

#include <QFileInfo>

#include <QFile>

#include <QList>

#include <QUrl>

#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void localDirCompare(const QString filepath);
private:
    QTimer *timer;
    QDateTime current_local_time;
    QFileSystemWatcher *fileWatcher;
    QString date_format;
    QDateTime getDateTimefromString(QString date_string);
    bool getFiles(QStringList getFileInfoList);
    bool FileWrite(QString filepath, QByteArray data);
    bool putFiles(QStringList putFileInfoList);
    bool deleteFiles(QStringList deleteFileInfoList);
    QString local_dir;
    WebDav *webdav;
    Sqlite *sqlite;
    Ui::MainWindow *ui;
private slots:
    void testing();
    void watcherFileChange(const QString filepath);
    void watcherDirChange(const QString filepath);
    void WebdavReplyFinished(QNetworkReply *reply);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
public slots:
};



#endif // MAINWINDOW_H
