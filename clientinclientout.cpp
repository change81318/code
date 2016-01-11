#include "clientinclientout.h"
#include <iostream>

ClientInClientOut::ClientInClientOut(QObject *parent) : QObject(parent)
{
     networkSession_ConnectiontoDst = NULL;
     networkSession_ConnectiontoSrc = NULL;
     tcpsocket_ConnectiontoSrc = NULL;
     tcpsocket_ConnectiontoDst = NULL;
     reconnecttimer = NULL;
     QNetworkConfigurationManager manager_connectiontoSrc;
     if (manager_connectiontoSrc.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
     {
         // Get saved network configuration
         QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
         settings.beginGroup(QLatin1String("QtNetwork"));
         const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
         settings.endGroup();

         // If the saved network configuration is not currently discovered use the system default
         QNetworkConfiguration config = manager_connectiontoSrc.configurationFromIdentifier(id);
         if ((config.state() & QNetworkConfiguration::Discovered) !=
             QNetworkConfiguration::Discovered) {
             config = manager_connectiontoSrc.defaultConfiguration();
         }

         networkSession_ConnectiontoSrc = new QNetworkSession(config, this);
         connect(networkSession_ConnectiontoSrc, SIGNAL(opened()), this, SLOT(ConnectiontoSrcsessionOpened()));
         networkSession_ConnectiontoSrc->open();
     }

     QNetworkConfigurationManager manager_connectiontoDst;
     if (manager_connectiontoDst.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
     {
         // Get saved network configuration
         QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
         settings.beginGroup(QLatin1String("QtNetwork"));
         const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
         settings.endGroup();

         // If the saved network configuration is not currently discovered use the system default
         QNetworkConfiguration config = manager_connectiontoDst.configurationFromIdentifier(id);
         if ((config.state() & QNetworkConfiguration::Discovered) !=
             QNetworkConfiguration::Discovered) {
             config = manager_connectiontoDst.defaultConfiguration();
         }

         networkSession_ConnectiontoDst = new QNetworkSession(config, this);
         connect(networkSession_ConnectiontoDst, SIGNAL(opened()), this, SLOT(ConnectiontoDstsessionOpened()));
         networkSession_ConnectiontoDst->open();
     }
     if (reconnecttimer == NULL)
     {
         reconnecttimer = new QTimer(this);
         connect(reconnecttimer, SIGNAL(timeout()), this, SLOT(Reconnect())/*, Qt::QueuedConnection*/);
         reconnecttimer->start(5000);
     }
}

void ClientInClientOut::ConnectiontoSrcsessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession_ConnectiontoSrc->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession_ConnectiontoSrc->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();
}

void ClientInClientOut::ConnectiontoDstsessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession_ConnectiontoDst->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession_ConnectiontoDst->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();
}

void ClientInClientOut::ConnecttoSrc(QHostAddress SrcServerIPAddress, int ServerPort)
{
    if (tcpsocket_ConnectiontoSrc && tcpsocket_ConnectiontoSrc->ConnectedState) return;
    SrcIPAddress = SrcServerIPAddress;
    iSrcServerPort = ServerPort;
    tcpsocket_ConnectiontoSrc = new QTcpSocket(this);
    tcpsocket_ConnectiontoSrc->abort();
    tcpsocket_ConnectiontoSrc->connectToHost(SrcServerIPAddress, iSrcServerPort);
    connect(tcpsocket_ConnectiontoSrc, SIGNAL(readyRead()),
            this, SLOT(ReadData_fromSrc()));
    connect(tcpsocket_ConnectiontoSrc, SIGNAL(disconnected()),
            this, SLOT(DisConnectedfromSrc()));
    connect(tcpsocket_ConnectiontoSrc, SIGNAL(connected()),
            this, SLOT(ConnectedtoSrc()));
    connect(tcpsocket_ConnectiontoSrc, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(ErrorHandler(QAbstractSocket::SocketError)));
}

void ClientInClientOut::ConnecttoDst(QHostAddress DstServerIPAddress, int ServerPort)
{
    if (tcpsocket_ConnectiontoDst && tcpsocket_ConnectiontoDst->ConnectedState) return;
    DstIPAddress = DstServerIPAddress;
    iDstServerPort = ServerPort;
    tcpsocket_ConnectiontoDst = new QTcpSocket(this);
    tcpsocket_ConnectiontoDst->abort();
    tcpsocket_ConnectiontoDst->connectToHost(DstServerIPAddress, iDstServerPort);
    connect(tcpsocket_ConnectiontoDst, SIGNAL(disconnected()),
            this, SLOT(DisConnectedfromDst())/*, Qt::DirectConnection*/);
    connect(tcpsocket_ConnectiontoDst, SIGNAL(disconnected()),
            tcpsocket_ConnectiontoDst, SLOT(deleteLater()));
    connect(tcpsocket_ConnectiontoDst, SIGNAL(connected()),
            this, SLOT(ConnectedtoDst()));
    connect(tcpsocket_ConnectiontoDst, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(ErrorHandler(QAbstractSocket::SocketError)));
}

void ClientInClientOut::ReadData_fromSrc()
{
    int bytesavailable = tcpsocket_ConnectiontoSrc->bytesAvailable();
 //   std::cout << "received " << bytesavailable << "bytes from Src Server" << std::endl;
    receiveddata = tcpsocket_ConnectiontoSrc->read(bytesavailable);
    if (tcpsocket_ConnectiontoDst)
    {
        QDateTime currenttime;
        tcpsocket_ConnectiontoDst->write(receiveddata);
        std::cout << "transmitted " << bytesavailable << "bytes from Src Server to Dst Server" << std::endl;
    }
}

void ClientInClientOut::DisConnectedfromSrc()
{
    std::cout << "Lost connection with Src Server IP: " << SrcIPAddress.toString().toStdString() << "port: " << iSrcServerPort << std::endl;
    tcpsocket_ConnectiontoSrc->abort();
    tcpsocket_ConnectiontoSrc = NULL;
}

void ClientInClientOut::DisConnectedfromDst()
{
//    QMutexLocker locker(&_mutex);
    std::cout << "Lost connection with Dst Server IP: " << DstIPAddress.toString().toStdString() << "port: " << iDstServerPort << std::endl;
    tcpsocket_ConnectiontoDst->abort();
    tcpsocket_ConnectiontoDst = NULL;

}

void ClientInClientOut::Reconnect()
{
   ConnecttoSrc(SrcIPAddress, iSrcServerPort);
   ConnecttoDst(DstIPAddress, iDstServerPort);
   int id = reconnecttimer->timerId();
   if (id) killTimer(id);
   reconnecttimer->start(5000);
}

void ClientInClientOut::ErrorHandler(QAbstractSocket::SocketError socketError)
{
    QTcpSocket *thesender = (QTcpSocket *)QObject::sender();
    thesender->abort();
    if( thesender == tcpsocket_ConnectiontoDst)
        tcpsocket_ConnectiontoDst = NULL;
    if( thesender == tcpsocket_ConnectiontoSrc)
        tcpsocket_ConnectiontoSrc = NULL;
//    tcpsocket_ConnectiontoDst->abort();
//    tcpsocket_ConnectiontoDst = NULL;
}

void ClientInClientOut::ConnectedtoSrc()
{
    std::cout << "Connected to Src Server: " << SrcIPAddress.toString().toStdString() << "port: " << iSrcServerPort << std::endl;
}

void ClientInClientOut::ConnectedtoDst()
{
    std::cout << "Connected to Dst Server: " << DstIPAddress.toString().toStdString() << "port: " << iDstServerPort << std::endl;
}
