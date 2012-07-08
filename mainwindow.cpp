#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    this->date_format = "ddd, dd MMM yyyy hh:mm:ss 'GMT'";

    this->local_dir = "/home/jeka/test";

    this->webdav = new WebDav(this, "davydov.evgeniy.ite@yandex.ru", "jkdavid1989_&");

//    this->timer = new QTimer(this);

    this->current_local_time = QDateTime::currentDateTime();

    connect(webdav->manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(WebdavReplyFinished(QNetworkReply*)));


    this->sqlite = new Sqlite;

    //    webdav->request("PROPPATCH", "qwe.txt", "");

    //watcher
    this->fileWatcher = new QFileSystemWatcher(this);
    this->fileWatcher->addPath(local_dir);


    //    watcher->addPath("test_dir/");

    //    watcher->addPath("test_dir/test_dir_2/");

    //    watcher->addPath("test_dir/test_dir_2/test_dir_3/");

    //    watcher->addPath("test_dir/qwe.txt");

    connect(this->fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherFileChange(const QString &)));

    connect(this->fileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherDirChange(const QString &)));
    //end watcher

    ui->setupUi(this);
}


void MainWindow::WebdavReplyFinished(QNetworkReply *reply) {
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkRequest request = reply->request();
    QString request_verb = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
    QByteArray output = this->webdav->replyFinished(reply);

    //global filepath
    QString custom_filepath = QUrl::fromPercentEncoding(request.rawHeader("custom_filepath"));

    QString local_dir_filepath = this->local_dir+custom_filepath;
    //end global filepath

    //PUT, MKCOL success
    if(status_code == 201) {
        //qDebug()<<"WEBDAV PUT REPLY"<<status_code<<getDateTimefromString(QString(reply->rawHeader("Date")));
    }

    if(request_verb == "PUT") {
        QFileInfo current_file(local_dir_filepath);
        current_file.setCaching(false);
        //        QDateTime reply_time = getDateTimefromString(reply->rawHeader("Date"));
        qDebug()<<"headers()"<<custom_filepath<<this->webdav->setProperty("local_last_modified", current_file.lastModified().toString(this->date_format).toLocal8Bit())->data();
        this->webdav->request("PROPPATCH", custom_filepath, this->webdav->setProperty("local_last_modified", current_file.lastModified().toString(this->date_format).toLocal8Bit())->data());
        qDebug()<<"put"<<local_dir_filepath;
    }

    if(request_verb == "GET") {
        FileWrite(QUrl::fromPercentEncoding(request.rawHeader("custom_filepath")), output);
        qDebug()<<"get"<<local_dir_filepath;
    }

    if(request_verb == "MKCOL") {
        this->webdav->request("PROPFIND", QUrl::fromPercentEncoding(request.rawHeader("custom_filepath")));
    }

    if(request_verb == "PROPFIND") {

        //        qDebug()<<"\noutput\n"<<output<<"\n";

        QStandardItemModel *remoteModel = this->webdav->propFindReply(output);

        QDir localDir = QDir(local_dir_filepath);

        localDir.setFilter(QDir::AllEntries | QDir::Hidden |QDir::NoDotAndDotDot);

        QFileInfoList localFileInfoList = localDir.entryInfoList(localDir.filter(), localDir.sorting());

        //file lists for change
        QStringList putFileInfoList;
        QStringList getFileInfoList;
        QStringList deleteFileInfoList;

        //local_files
        for(int i = 0; i < localFileInfoList.count(); i++){
            QString local_filename_suffix = "";
            QFileInfo local_file = localFileInfoList.at(i);
            //            qDebug()<<"filepath"<<local_file.filePath();
            this->fileWatcher->addPath(local_file.filePath());
            local_file.setCaching(false);
            if(local_file.isDir()) {
                local_filename_suffix = "/";
            }
            QString local_filename = custom_filepath+local_file.fileName()+local_filename_suffix;
            bool is_find = false;
            int find_index = 0;

            for(int j = 0; j < remoteModel->rowCount(); j++) {
                QString remote_filename = remoteModel->data(remoteModel->index(j, 0, QModelIndex()), Qt::DisplayRole).toString();
                if(local_filename == remote_filename) {
                    find_index = j;
                    is_find = true;
                    break;
                }
            }
            if(is_find){
                qDebug()<<local_filename<<remoteModel->data(remoteModel->index(find_index, 0, QModelIndex()), Qt::DisplayRole).toString();
                qDebug()<<local_filename<<remoteModel->data(remoteModel->index(find_index, 1, QModelIndex()), Qt::DisplayRole).toString();
                qDebug()<<local_filename<<remoteModel->data(remoteModel->index(find_index, 2, QModelIndex()), Qt::DisplayRole).toString();
                qDebug()<<local_filename<<remoteModel->data(remoteModel->index(find_index, 3, QModelIndex()), Qt::DisplayRole).toString();
                qDebug()<<local_filename<<remoteModel->data(remoteModel->index(find_index, 4, QModelIndex()), Qt::DisplayRole).toString();

                QDateTime remote_model_time = getDateTimefromString(remoteModel->data(remoteModel->index(find_index, 3, QModelIndex()), Qt::DisplayRole).toString());
                QString local_last_modified = remoteModel->data(remoteModel->index(find_index, 4, QModelIndex()), Qt::DisplayRole).toString();

                qDebug()<<"compare"<<local_filename<<local_file.lastModified().toString(this->date_format) << local_last_modified;

                if(remote_model_time < local_file.lastModified() && local_file.lastModified().toString(this->date_format) != local_last_modified) {
                    putFileInfoList.append(local_filename);
                    //qDebug()<<"local"<<"<"<<local_filename;
                }
            }
            else {
                //NOT FIND ON REMOTE FILES
                //qDebug()<<"NOT FOUND IN REMOTE"<<local_filename;
                putFileInfoList.append(local_filename);
            }
            //            local_file.~QFileInfo();
        }
        //remote files
        for(int j = 0; j < remoteModel->rowCount(); j++) {
            bool is_find = false;
            int find_index = 0;
            QString remote_filename = remoteModel->data(remoteModel->index(j, 0, QModelIndex()), Qt::DisplayRole).toString();
            for(int i = 0; i < localFileInfoList.count(); i++){
                QString local_filename_suffix = "";
                QFileInfo local_file = localFileInfoList.at(i);
                local_file.setCaching(false);
                if(local_file.isDir()) {
                    local_filename_suffix = "/";
                }
                QString local_filename = custom_filepath+local_file.fileName()+local_filename_suffix;
                if(local_filename == remote_filename) {
                    find_index = i;
                    is_find = true;
                    break;
                }
                //                local_file.~QFileInfo();
            }
            QString local_last_modified = remoteModel->data(remoteModel->index(j, 4, QModelIndex()), Qt::DisplayRole).toString();
            if(is_find) {
                QFileInfo local_file = localFileInfoList.at(find_index);
                local_file.setCaching(false);
                QDateTime remote_model_time = getDateTimefromString(remoteModel->data(remoteModel->index(j, 3, QModelIndex()), Qt::DisplayRole).toString());

                if(remote_model_time > local_file.lastModified() && local_file.lastModified().toString(this->date_format) != local_last_modified) {
                    getFileInfoList.append(remote_filename);
                    //qDebug()<<"remote"<<">"<<remote_filename;
                }
                //                local_file.~QFileInfo();
            }
            else {
                //NOT FIND ON LOCAL FILES
                qDebug()<<"local_last_modified"<<remote_filename<<local_last_modified;
                if(local_last_modified == "") {
                    getFileInfoList.append(remote_filename);
                }
                else {
                    deleteFileInfoList.append(remote_filename);
                }
                //qDebug()<<"NOT FOUND IN LOCAL"<<remote_filename;
            }
        }
        putFiles(putFileInfoList);
        getFiles(getFileInfoList);
        deleteFiles(deleteFileInfoList);
    }
    this->webdav->testing();
}

bool MainWindow::putFiles(QStringList putFileInfoList) {
    if(putFileInfoList.count() > 0) {
        for(int i = 0; i < putFileInfoList.count(); i++){

            QString current_file = this->local_dir + putFileInfoList.at(i);

            if(current_file.lastIndexOf("/") == current_file.length()-1 && current_file != "") {

                //qDebug()<<"putFiles-directory"<<putFileInfoList.at(i);

                this->webdav->request("MKCOL", putFileInfoList.at(i));

                //                webdav->request("PROPFIND", putFileInfoList.at(i));
            }
            else {
                QFile file(current_file);

                //qDebug()<<"putFiles-file"<<putFileInfoList.at(i);

                file.open(QIODevice::ReadOnly);

                this->webdav->request("PUT", putFileInfoList.at(i), file.readAll());
            }
        }
    }
}

bool MainWindow::getFiles(QStringList getFileInfoList) {
    if(getFileInfoList.count() > 0) {
        for(int i = 0; i < getFileInfoList.count(); i++){
            this->webdav->request("GET", getFileInfoList.at(i));
        }
    }
}

bool MainWindow::FileWrite(QString filepath, QByteArray data) {

    QString current_filepath = this->local_dir + filepath;

    if(current_filepath.lastIndexOf("/") == current_filepath.length()-1 && current_filepath != "") {
        QDir dir;
        dir.mkdir(current_filepath);
        webdav->request("PROPFIND", filepath);

        //qDebug()<<"FileWrite-directory"<<filepath;
    }
    else {
        //qDebug()<<"FileWrite-file"<<filepath;
        QFile file(current_filepath);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }

    this->fileWatcher->addPath(current_filepath);
    QFileInfo current_file(current_filepath);
    current_file.setCaching(false);
    this->webdav->request("PROPPATCH", filepath, this->webdav->setProperty("local_last_modified", current_file.lastModified().toString(this->date_format).toLocal8Bit())->data());
}

bool MainWindow::deleteFiles(QStringList deleteFileInfoList) {
    qDebug()<<"deleteFileInfoList"<<deleteFileInfoList;
    for(int i = 0; i< deleteFileInfoList.length(); i++ ) {
        this->webdav->request("DELETE", deleteFileInfoList.at(i));
    }
}

QDateTime MainWindow::getDateTimefromString(QString date_string) {
    QDateTime dateTime = QDateTime::fromString(date_string, this->date_format).toLocalTime();
    dateTime.setTimeSpec(Qt::UTC);
    return dateTime;
}

void MainWindow::watcherDirChange(const QString filepath){

//    this->fileWatcher->

    QDateTime tmp_current_time = QDateTime::currentDateTime();
//    tmp_current_time.currentDateTime();

    qDebug()<<"time"<<tmp_current_time.toTime_t()<<current_local_time.toTime_t();

//    sleep();

    if(tmp_current_time.toTime_t() > current_local_time.toTime_t() + 10) {
        QString current_filepath = filepath;
        current_filepath.replace(0, this->local_dir.length(), "");
        current_filepath += "/";
//        this->webdav->request("PROPFIND", current_filepath);
        qDebug()<<"file_changed"<<current_filepath;
    }

    this->localDirCompare(filepath);

    this->current_local_time = QDateTime::currentDateTime();
//    qDebug()<<"current_local_time"<<current_local_time.toTime_t();
//        qDebug()<<"networkAccessible"<<this->webdav->manager->networkAccessible();
}

void MainWindow::watcherFileChange(const QString filepath){
    QStringList watcherFiles = this->fileWatcher->files();
    QString current_filepath = filepath;
    current_filepath.replace(0, this->local_dir.length(), "");
//    current_filepath += "/";

    if(watcherFiles.contains(filepath)) {
//        this->webdav->request("PROPFIND", current_filepath);
    }
    else {
//        this->webdav->request("DELETE", current_filepath);
    }
//    qDebug()<<"watcherFileChange"<<filepath;
}

void MainWindow::localDirCompare(const QString filepath){
    QDir localDir = QDir(filepath);
    localDir.setFilter(QDir::AllEntries | QDir::Hidden |QDir::NoDotAndDotDot);
    QFileInfoList localFileInfoList = localDir.entryInfoList(localDir.filter(), localDir.sorting());

    QStringList watcherFiles = this->fileWatcher->files();

    foreach(QFileInfo fileInfo, localFileInfoList) {

//        qDebug()<<"localFileInfoList"<<fileInfo.filePath();
    }

    qDebug()<<"watcherDirChange"<<filepath;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    this->webdav->request("PROPFIND");
}

void MainWindow::on_pushButton_2_clicked()
{

    QString filepath = "qwerty";

    QTimer::singleShot(2000, this->parent(), SLOT(testing()));

//    qDebug()<<"WATCHER FILES"<<this->fileWatcher->files();
//    qDebug()<<"WATCHER DIRECTORIES"<<this->fileWatcher->directories();
}

void MainWindow::testing(){
    qDebug()<<"timer_event";
}
