#include "mainwindow.h"
#include "networkThread.h"
#include <iostream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow *w = new MainWindow();
    NetworkThread *net = new NetworkThread(w);

    //Setup Error Messaging
    w->connect(w,SIGNAL(onError(QString, QString)),w,SLOT(printError(QString, QString)));
    w->connect(net,SIGNAL(onError(QString, QString)),w,SLOT(printError(QString, QString)));

    w->connect(&a,SIGNAL(aboutToQuit()),w,SLOT(closeUI()));
    w->show();

    //Attempt to connect
    if(net->connectNet("192.168.10.1")){
        w->connect(w,SIGNAL(dataWriten(QString,QString)),net, SLOT(writeNet(QString,QString)));
        w->connect(net,SIGNAL(dataRead(QString)), w, SLOT(dataParse(QString)));
        w->connect(&a,SIGNAL(aboutToQuit()),net,SLOT(closeNet()));
    }
    else{
        w->printError("Timeout","Client unable to connect to server");
    }



    return a.exec();
}
