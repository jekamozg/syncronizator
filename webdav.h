#ifndef WEBDAV_H
#define WEBDAV_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QString>
#include <QFile>
#include <QBuffer>
#include <QByteArray>
#include <QObject>

#include <QDomDocument>

#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QModelIndex>

#include <QStandardItemModel>

#include <QFileInfo>



//#include "qsql_sqlite.h"

//#include <qxml

//#include <QIODevice>

class WebDav : public QObject
{
    Q_OBJECT

public:
    QStandardItemModel* propFindReply(QByteArray replydata);
    QByteArray replyFinished(QNetworkReply*);
    QNetworkAccessManager *manager;
    void testing();
    WebDav(QObject * parent = 0, QString login = "", QString password = "");
    void request(QString verb);
    void request(QString verb, QString filepath);
    void request(QString verb, QString filepath, const QByteArray data);

    ~WebDav();

    QByteArray *setProperty(QString property, QString value, QString namespc = "synchronizator");
    QByteArray *getProperty(QString property, QString namespc = "synchronizator");
private:
//    Ui::WebDav *ui;
    QString login;
    QString password;
    QStringList testing_sl;
    QStandardItemModel *all_remote_files;
    QString encodeBase64(QString string);
    void setHeaders(QNetworkRequest & request);
    void setHeaders(QNetworkRequest & request, const int size);
    void setHeaders(QNetworkRequest & request, bool depth);
    void setHeaders(QNetworkRequest & request, const int size, bool depth);
//    void replyFinished(QNetworkReply*);


private slots:
//    void pushButtonClicked();
};

#endif // WEBDAV_H
