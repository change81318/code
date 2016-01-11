#include <QCoreApplication>
#include "clientinclientout.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString pfilepath;
    QString qsSrcServerAddress, qsDstServerAddress;
    int iSrcServerPort, iDstServerPort;
    pfilepath = QCoreApplication::applicationDirPath();
    QString pfilename = pfilepath + "/clientinclientout.txt";
    QFile pfile(pfilename);
    if (pfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&pfile);
        QString line;
        line = in.readLine();
        qsSrcServerAddress = line;
        line = in.readLine();
        iSrcServerPort = line.toInt();
        line = in.readLine();
        qsDstServerAddress = line;
        line = in.readLine();
        iDstServerPort = line.toInt();
        pfile.close();
    }
    QHostAddress SrcServerAddress(qsSrcServerAddress);
    QHostAddress DstServerAddress(qsDstServerAddress);
    ClientInClientOut clientinclientout;
    clientinclientout.ConnecttoSrc(SrcServerAddress, iSrcServerPort);
    clientinclientout.ConnecttoDst(DstServerAddress, iDstServerPort);
    return a.exec();
}

