#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include <QString>
#include <QThread>
#include <QtNetwork>
#include <QTcpSocket>

class NetworkThread : public QUdpSocket
{
    Q_OBJECT
public:
    NetworkThread(QWidget *parent);
    ~NetworkThread();
    bool connectNet(QString ip);

signals:
    void dataRead(QString buffer);
    void onError(QString title, QString err);

public slots:
    void readNet();
    void writeNet(QString f_up, QString f_down);
    void closeNet();

private:
    QHostAddress *server;
    QTimer *timer;

};


#endif // NETWORKTHREAD_H
