#ifndef CLIENTINCLIENTOUT_H
#define CLIENTINCLIENTOUT_H

#include <QObject>
#include <QtNetwork/QtNetwork>

class ClientInClientOut : public QObject
{
    Q_OBJECT
public:
    explicit ClientInClientOut(QObject *parent = 0);

signals:

public slots:
    void ConnectiontoSrcsessionOpened();
    void ConnectiontoDstsessionOpened();
    void ConnecttoSrc(QHostAddress, int);
    void ConnecttoDst(QHostAddress, int);
    void ReadData_fromSrc();
    void DisConnectedfromDst();
    void DisConnectedfromSrc();
    void Reconnect();
    void ConnectedtoSrc();
    void ConnectedtoDst();
    void ErrorHandler(QAbstractSocket::SocketError socketError);


private:
    QTcpSocket *tcpsocket_ConnectiontoSrc;
    QTcpSocket *tcpsocket_ConnectiontoDst;
    QNetworkSession *networkSession_ConnectiontoSrc;
    QNetworkSession *networkSession_ConnectiontoDst;

    QByteArray receiveddata;
    QHostAddress SrcIPAddress;
    QHostAddress DstIPAddress;
    int iSrcServerPort;
    int iDstServerPort;

    QTimer * reconnecttimer;
    QMutex _mutex;
};

#endif // CLIENTINCLIENTOUT_H
