#include "networkThread.h"
#include <QDebug>

NetworkThread::NetworkThread(QWidget *parent){
    server = new QHostAddress();
    timer = new QTimer(this);
    timer->setSingleShot(true);
}

NetworkThread::~NetworkThread(){
    //Free pointers
    delete server;
    delete timer;
}

bool NetworkThread::connectNet(QString ip){
    //Send connection message to requested IP
    QByteArray sendGram = "Con";
    server->setAddress(ip);
    writeDatagram(sendGram.data(), sendGram.size(), *server, 9001);

    //wait for acceptance
    timer->start(1000);
    while (timer->remainingTime() != 0 && !hasPendingDatagrams()){}
    if(!hasPendingDatagrams()){ //If timer ran out before we got data, error
        return false;
    }
    sendGram.resize(pendingDatagramSize());
    readDatagram(sendGram.data(), sendGram.size());

    //connected, we good to go!
    connect(this, SIGNAL(readyRead()), this, SLOT(readNet()));
    return true;

}

void NetworkThread::readNet(){
    while (hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(pendingDatagramSize());
            readDatagram(datagram.data(), datagram.size());
            QString out(datagram.constData());
            emit dataRead(out); //Tell the ui we read data
    }
}

void NetworkThread::writeNet(QString f_up, QString f_down){
    QByteArray sendGram = f_down.toLocal8Bit() + " " + f_up.toLocal8Bit();
    writeDatagram(sendGram.data(), sendGram.size(), *server, 9001);
    QString out(sendGram.constData());
}

void NetworkThread::closeNet(){
    //Send the server the disconnect command
    QByteArray sendGram = "Dis";
    writeDatagram(sendGram.data(), sendGram.size(), *server, 9001);
    qDebug() << "Disconnect Sent";
}
