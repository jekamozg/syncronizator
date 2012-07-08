#ifndef SQLITE_H
#define SQLITE_H

#include <QObject>

#include <QSqlDatabase>
#include <QSqlQuery>

#include <QStringList>
#include <QSqlRecord>

#include <QThread>

class Sqlite : public QObject
{
    Q_OBJECT
public:
    QSqlDatabase db;
    explicit Sqlite(QObject *parent = 0);
    ~Sqlite();
    bool addFile(QString filepath, QString remote_last_time, QString local_last_time);
    QStringList getFile(QString filepath);
private:
    bool init();
signals:
    
public slots:
    
};

#endif // SQLITE_H
