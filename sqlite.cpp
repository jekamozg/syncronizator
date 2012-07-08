#include "sqlite.h"
#include <QDebug>

Sqlite::Sqlite(QObject *parent) :
    QObject(parent)
{


//    random();

//    //qDebug()<<"QThread::currentThreadId();"<</*parent->parent()->thread()->currentThreadId()*/;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setConnectOptions();
    db.setDatabaseName("qwe.db");
    db.open();
    this->init();
}

bool Sqlite::init() {
    db.exec("CREATE TABLE files (filepath varchar(1024), remote_last_time varchar(64), local_last_time varchar(64), PRIMARY KEY (filepath))");
    //    db.close();
}

bool Sqlite::addFile(QString filepath, QString remote_last_time, QString local_last_time) {
    QString values = "'"+filepath+"'"+","+"'"+remote_last_time+"'"+","+"'"+local_last_time+"'";
    db.exec("INSERT OR REPLACE INTO files (filepath, remote_last_time, local_last_time) VALUES("+values+")");
}


QStringList Sqlite::getFile(QString filepath) {
    QStringList output;
    QSqlQuery result;
    QSqlRecord rec;

    result = db.exec("SELECT * FROM files WHERE filepath='"+filepath+"'");

    rec = result.record();

    int filepathCol = rec.indexOf("filepath");
    int remote_last_timeCol = rec.indexOf("remote_last_time");
    int local_last_timehCol = rec.indexOf("local_last_time");
    int i = 0;
    while(result.next()) {
        if(i == 0) {
            output<<result.value(filepathCol).toString();
            output<<result.value(remote_last_timeCol).toString();
            output<<result.value(remote_last_timeCol).toString();
        }
        i++;
    }
    //qDebug()<<"sqlite find"<<output;
    return output;
}

Sqlite::~Sqlite()
{
    db.close();
}



