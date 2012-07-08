#include "webdav.h"
#include <QDebug>

WebDav::WebDav(QObject * parent, QString login, QString password) : manager(new QNetworkAccessManager(parent))
{
    this->login = login;
    this->password = password;
//    this->all_remote_files = new QStandardItemModel(0, 4, parent);
//    connect(manager, SIGNAL(finished(QNetworkReply*)),
//            this, SLOT(replyFinished(QNetworkReply*)));
}

void WebDav::testing() {
    qDebug()<<"webdav_testing";
}

void WebDav::request(QString verb){
    request(verb, "/", "");
}

void WebDav::request(QString verb, QString filepath) {
    request(verb, filepath, "");
}

void WebDav::request(QString verb, QString filepath, const QByteArray data){
    QNetworkRequest request;
    QBuffer *buffer = new QBuffer(this->parent());
    buffer->setData(data);
    request.setUrl(QUrl("https://webdav.yandex.ru"+filepath));
    qDebug()<<"request.url();"<<request.url();
    verb = verb.toUpper();
    request.setRawHeader("custom_filepath", filepath.toLocal8Bit());
    if(verb == "PUT") {
        setHeaders(request, data.length());
    }
    else if(verb == "GET") {
        setHeaders(request);
    }
    else if(verb == "MKCOL"){
        setHeaders(request, data.length());
    }
    else if(verb == "DELETE"){
        setHeaders(request, data.length());
    }
    else if(verb == "PROPFIND") {
        buffer->setData("<propfind xmlns='DAV:'><prop><local_last_modified xmlns='synchronizator'/><getcontentlength/><getlastmodified/><creationdate/></prop></propfind>");
        setHeaders(request, true);
    }

    else if(verb == "PROPPATCH") {
        setHeaders(request, true);
    }

    else if(verb == "PROPFINDALL") {
        setHeaders(request, true);
        request.setRawHeader("PROPFINDALL", "enabled");
        verb = "PROPFIND";
    }
    else {
        //////qDebug()<<"not allowed verb";
    }
    manager->sendCustomRequest(request, verb.toLocal8Bit(), buffer);
}
void WebDav::setHeaders(QNetworkRequest & request) {
    setHeaders(request, 0, false);
}

void WebDav::setHeaders(QNetworkRequest & request, bool depth) {
    setHeaders(request, 0, depth);
}

void WebDav::setHeaders(QNetworkRequest & request, const int size) {
    setHeaders(request, size, false);
}

void WebDav::setHeaders(QNetworkRequest & request, const int size, bool depth) {
    QString headerData = "Basic "+encodeBase64(this->login+":"+this->password);

    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    if (size > 0) {
        request.setRawHeader("Content-Type", "application/binary");
        request.setRawHeader("Expect", "100-continue");
        //testing
//        request.setRawHeader("Last-Modified","Sun, 20 Mar 2005 12:00:00 GMT");
        //testing
        request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(size));
    }
    if (depth){
        request.setRawHeader("Depth", "1");
    }
    //////qDebug()<<request.rawHeaderList();
}

QByteArray WebDav::replyFinished(QNetworkReply *reply) {
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();



    QNetworkRequest request = reply->request();

    QString request_verb = request.attribute(QNetworkRequest::CustomVerbAttribute).toString();

    QByteArray output = reply->readAll();

    //reply debug
    //qDebug()<<"output:"<<status_code;
    //qDebug()<<"URL"<<request.url();
    //qDebug()<<"request.rawHeader(custom_filepath)"<<reply->rawHeaderPairs();
    //end reply debug

    if(request_verb == "PUT") {

    }
    else if(request_verb == "GET") {

    }
    else if(request_verb == "MKCOL"){

    }
    else if(request_verb == "PROPFIND") {
        //qDebug()<<"output\n"<<output<<"\n";
        //////qDebug()<<remote_files->data(remote_files->index(0, 1, QModelIndex()), Qt::DisplayRole);
        //////qDebug()<<remote_files->rowCount();
        //////qDebug()<<remote_files->columnCount();

//        reply->rawHeaderPairs();

        if(request.rawHeader("PROPFINDALL") == "enabled") {
            QStandardItemModel *remote_files = propFindReply(output);
            int current_row_count = remote_files->rowCount();
            ////qDebug()<<"current_row_count"<<current_row_count;
            for(int i = 0; i < current_row_count; i++) {
                QString current_file = remote_files->data(remote_files->index(i, 0, QModelIndex()), Qt::DisplayRole).toString();
                ////qDebug()<<"all_files"<<current_file;
                testing_sl.append(current_file);
                if(current_file.lastIndexOf("/") == current_file.length()-1 && current_file != "") {
                    this->request("PROPFINDALL",current_file);
                }
            }
        }
    }
    return output;
}

QStandardItemModel* WebDav::propFindReply(QByteArray replydata){
    QDomDocument xml;
    QDomNodeList output;
    QStringList xml_list;
    QString prefix = "d:";

    xml_list.append("href");
    xml_list.append("creationdate");
    xml_list.append("getcontentlength");
    xml_list.append("getlastmodified");
    xml_list.append("local_last_modified");

    xml.setContent(replydata);

    int test = 0;

    QStandardItemModel *remote_files = new QStandardItemModel(0, xml_list.count(), this->parent());

    for(int i = 0; i < xml_list.length(); i++){
        QString tag_name;
        if(i == 4) {
            tag_name = xml_list.at(i);
        }
        else {
            tag_name = prefix+xml_list.at(i);
        }
        output = xml.elementsByTagName(tag_name);
        if(i == 0) {
            remote_files->insertRows(0, output.length()-1, QModelIndex());
        }
        for(unsigned int j = 1; j < output.length(); j++){
            QString element_text = output.at(j).toElement().text();
            if(i == 0) {
                element_text = QUrl::fromPercentEncoding(output.at(j).toElement().text().toLocal8Bit());
            }
            remote_files->setData(remote_files->index(j-1, i, QModelIndex()), QVariant(element_text), Qt::EditRole);
            qDebug()<<"item_data"<<tag_name<<element_text;
            test++;
        }
    }
    return remote_files;
}

QByteArray* WebDav::setProperty(QString property, QString value, QString namespc) {
    QByteArray *output = new QByteArray;
    QDomDocument doc;
    QDomElement root = doc.createElement("propertyupdate");
    root.setAttribute("xmlns", "DAV:");
    root.setAttribute("xmlns:u", namespc);
    doc.appendChild(root);

    QDomElement set = doc.createElement("set");
    root.appendChild(set);

    QDomElement prop = doc.createElement("prop");
    set.appendChild(prop);

    QDomElement myprop = doc.createElement("u:"+property);
    prop.appendChild(myprop);

    QDomText t = doc.createTextNode(value);
    myprop.appendChild(t);

    output->append(doc.toString());
    return output;
}

QByteArray* WebDav::getProperty(QString property, QString namespc) {
    QByteArray *output = new QByteArray;
    QDomDocument doc;
    QDomElement root = doc.createElement("propfind");
    root.setAttribute("xmlns", "DAV:");
    doc.appendChild(root);

    QDomElement prop = doc.createElement("prop");
    root.appendChild(prop);

    QDomElement myprop = doc.createElement(property);
    myprop.setAttribute("xmlns", namespc);
    prop.appendChild(myprop);

    output->append(doc.toString());

    return output;
}

QString WebDav::encodeBase64( QString string ) {
    QByteArray text;
    text.append(string);
    return text.toBase64();
}

WebDav::~WebDav()
{
    delete manager;
}
