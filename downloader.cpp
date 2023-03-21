#include "downloader.h"

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    // Initialize manager ...
    manager = new QNetworkAccessManager();
    // ... and connect the signal to the handler
    connect(manager, &QNetworkAccessManager::finished, this, &Downloader::onResult);
}

void Downloader::getData(const QString& urlstr)
{
    QUrl url(urlstr);
    QNetworkRequest request(url);

    if(url.scheme()=="https")
    {
        QSslConfiguration conf = request.sslConfiguration();
        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(conf);
    }

    manager->get(request);
}

void Downloader::onResult(QNetworkReply *reply)
{
    // If an error occurs in the process of obtaining data
    if(reply->error()){
        // We inform about it and show the error information
        qDebug() << "ERROR";
        qDebug() << reply->errorString();
    } else {
        // Otherwise we create an object file for use with
        QFile *file = new QFile("/home/zoli/oui.txt");
        // Create a file, or open it to overwrite ...
        if(file->open(QFile::WriteOnly)){
            file->write(reply->readAll());  // ... and write all the information from the page file
            file->flush();
            file->close();                  // close file
        qDebug() << "Downloading is completed";
        emit onReady(); // Sends a signal to the completion of the receipt of the file
        }
    }
}
