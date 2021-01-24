#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <QObject>
#include <QtNetwork>
#include <qtconcurrentrun.h>
#include <QThread>

QString static url;

class WebHook : public QObject
{
    Q_OBJECT
public:
    explicit WebHook(QString urlC)
    {
        url = urlC;
    }

    void sendMsg(QByteArray json)
    {
        QFuture<void> f1 = QtConcurrent::run(printMsg, json);
    }

private:
    static void printMsg(QByteArray json)
    {
        QNetworkRequest req(url);
        QNetworkAccessManager nam;
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=utf-8");
        QNetworkReply *reply = nam.post(req, json);
        while (!reply->isFinished())
        {
            qApp->processEvents();
        }
    }
};

#endif // WEBHOOK_H
