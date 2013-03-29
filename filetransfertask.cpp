#include <sys/types.h>
#include <unistd.h>

#include <QApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkInterface>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <kdebug.h>
#include <kopete/kopetetransfermanager.h>

#include "debug.h"
#include "mra/mraprotocolv123.h"
#include "mra/transferrequestinfo.h"
#include "mrimcontact.h"
#include "mrimaccount.h"
#include "filetransfertask.h"

enum state {
    NOT_CONNECTED,
    CONNECTED,
    HELLO_RECEIVED

};

/**
 *TODO:
 * - cancel transfer by remote client
 * - cancel transfer by user (via tray)
 * - try to open channel to remote client when it asks
 */

struct FileTransferTask::Private {
    MrimAccount *account;
    MRAProtocolV123 *proto;
    MrimContact *contact;
    QTcpServer *server;
    QTcpSocket *socket;
    int bytesSent;
    QStringList fileNames;
    Kopete::Transfer *tranfserTask;
    QFile *file;
    FileTransferTask::Direction dir;
    TransferRequestInfo transferInfo;

    QList<QPair<QString, int> > files;

    Private()
        : account(0)
        , proto(0)
        , contact(0)
        , server(0)
        , socket(0)
        , bytesSent(0)
        , tranfserTask(0)
        , file(0) {
    }
};

FileTransferTask::FileTransferTask(MrimAccount *account,
        MrimContact *contact,
        QStringList fileNames,
        Direction dir,
        const TransferRequestInfo *info,
        QObject *parent)
    : QObject(parent)
    , d(new Private)

{
    d->account = account;
    d->contact = contact;
    d->fileNames = fileNames;
    d->dir = dir;


    d->proto = dynamic_cast<MRAProtocolV123 *>(account->getMraProtocol());

    kDebug(kdeDebugArea()) << "starting";

    QApplication::processEvents();

    if (d->dir == Outgoing) {
        d->file = new QFile(fileNames[0], this);

        d->tranfserTask = Kopete::TransferManager::transferManager()
                ->addTransfer(contact, fileNames[0], d->file->size(), contact->contactId(), Kopete::FileTransferInfo::Outgoing);


        connect(this, SIGNAL(bytesSent(uint)),
                d->tranfserTask, SLOT(slotProcessed(uint)));

        connect(this, SIGNAL(transferComplete()),
                d->tranfserTask, SLOT(slotComplete()));
        openServer();
    } else {

        connect ( Kopete::TransferManager::transferManager (), SIGNAL (accepted(Kopete::Transfer*,QString)),
                  this, SLOT (slotTransferAccepted(Kopete::Transfer*,QString)) );

        connect ( Kopete::TransferManager::transferManager (), SIGNAL (refused(Kopete::FileTransferInfo)),
                  this, SLOT (slotTransferRefused(Kopete::FileTransferInfo)) );

        d->transferInfo = *info;
        Kopete::TransferManager::transferManager()
                ->askIncomingTransfer(contact, d->transferInfo.getFiles()[0].first, d->transferInfo.totalSize());


    }

}

FileTransferTask::~FileTransferTask() {
    delete d;
}

void FileTransferTask::openServer() {

    d->server = new QTcpServer(this);

    connect(d->server, SIGNAL(newConnection()),
            this, SLOT(slotIncomingConnection()) );


    d->server->listen(QHostAddress("192.168.1.43"), 2041);

    d->proto->startFileTransfer(this);
}

void FileTransferTask::openSocket(const TransferRequestInfo *info) {
    d->socket = new QTcpSocket(this);
    QPair<QString, int> connectData = info->getHostsAndPorts()[0];
    d->files = info->getFiles();

    d->socket->connectToHost(connectData.first, connectData.second);
    d->socket->waitForConnected(5000);
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotReadIncommingDataClient()));
    QByteArray hello;
    hello.append("MRA_FT_HELLO ");
    hello.append(d->account->accountId());
    hello.append('\0');
    d->socket->write(hello);
}

void FileTransferTask::slotReadIncommingDataClient() {
    const QString hello = "MRA_FT_HELLO " + getContact() + QChar('\0');
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray data = socket->readAll();
    if (data.startsWith("MRA_FT_HELLO ")) {
        QByteArray getFile;
        getFile.append("MRA_FT_GET_FILE ");
        getFile.append(d->files[0].first);
        getFile.append('\0');

        socket->write(getFile);
    } else {
        // file data
        QString filename;
        if (d->tranfserTask->info().saveToDirectory()) {
            mrimDebug() << "save to directory";

        }
        filename = d->fileNames[0];
        mrimDebug() << "filename" << filename;
        QFile file(filename);
        file.open(QIODevice::WriteOnly); // TODO: check and report error

        file.write(data);
        int bytesReceived = data.size();
        kDebug(kdeDebugArea()) << "bytes" << bytesReceived;

        const int bytsTotal = d->files[0].second;
        while(bytesReceived < bytsTotal) {
            QApplication::processEvents();

            if ( socket->waitForReadyRead(100) ) {
                QByteArray chunk = socket->readAll(); /// @todo errors check
                file.write(chunk);
                bytesReceived += chunk.size();

                kDebug(kdeDebugArea()) << "chunk.size" << chunk.size();
                d->tranfserTask->slotProcessed(bytesReceived);
            } else {
                kDebug(kdeDebugArea()) << "no data";
            }

        }
        finishTransfer(true);
    }
}

QString FileTransferTask::getContact() {
    return d->contact->contactId();
}

QString FileTransferTask::getAccountId() {
    return d->account->accountId();
}

QString FileTransferTask::getFilePath() {
    return QFileInfo(d->file->fileName()).fileName(); // "tar.gz"; // 69925;";
}

int FileTransferTask::getFileSize() {
    return d->file->size();
}

int FileTransferTask::getSessionId() {
    static int i = 0;
    return getpid() + i++;
}

QString FileTransferTask::getHostAndPort() {
    d->server->serverAddress();

    QNetworkInterface inter;

    QString str;

    foreach (const QHostAddress &a, inter.allAddresses()) {
        if (a == QHostAddress::LocalHost ||
            a == QHostAddress::Null ||
            a.protocol() == QAbstractSocket::IPv6Protocol
                ) {
            continue;
        }
        str += a.toString() + ":2041;"; /// @todo port to member
    }

    return str;
}

void FileTransferTask::slotIncomingConnection() {
    kDebug(kdeDebugArea()) << "new connection" ;
    QTcpSocket* s = d->server->nextPendingConnection();
    QObject::connect(s, SIGNAL(readyRead()), this, SLOT(slotReadOutgoingDataClient()));
    QObject::connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

void FileTransferTask::discardClient() {

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void FileTransferTask::slotReadOutgoingDataClient()
{
    // MRA_FT_HELLO mikhail.galanin@bk.ru.
    const QString hello = "MRA_FT_HELLO " + getContact() + QChar('\0');

    QTcpSocket* socket = (QTcpSocket*)sender();

    // TODO: wait for full line
    /*
    while( not socket->canReadLine() && socket->isValid() ) {
        socket->waitForReadyRead(100);
        QApplication::processEvents();
    }
    */

    QByteArray data = socket->readAll();

    kDebug(kdeDebugArea()) << "awaited" << hello;
    kDebug(kdeDebugArea()) << "size" << hello.length();
    kDebug(kdeDebugArea()) << "got" << QString(data) << data.length();

    if (data.startsWith("MRA_FT_HELLO ")) {

        QByteArray recvData;

        recvData.append("MRA_FT_HELLO " + d->account->accountId() + '\0');

        kDebug(kdeDebugArea()) << "written" << socket->write(recvData);

    } else {

        socket->readAll();

        // "MRA_FT_GET_FILE file_name\0"

        if ( !d->file->open(QIODevice::ReadOnly) ) {
            kDebug(kdeDebugArea()) << "shit";
            return;
        }

        QByteArray fileData = d->file->readAll();

        if (socket->write(fileData) == -1) {
            // TODO: report error
        }

        while( socket->bytesToWrite() > 0 ) {

            QApplication::processEvents();

            if ( socket->waitForBytesWritten(100) ) {
                uint bytesWritten = (fileData.size() - d->bytesSent) - socket->bytesToWrite();
                d->bytesSent += bytesWritten;
                kDebug(kdeDebugArea()) << "bytes sent: " << bytesWritten;

                emit bytesSent(d->bytesSent);
            } else if (not socket->isOpen() ) {
                kDebug(kdeDebugArea()) << "error: ";
                // emit error
                break; // error occured
            }
            // kDebug(kdeDebugArea()) << "keep calm: " << bytesWritten;

        }

        socket->close();

        finishTransfer(true);
    }

}

void FileTransferTask::finishTransfer(bool succesful) {
    if (d->server) {
        d->server->close();
    } else if (d->socket) {
        d->socket->close();
    }
    if (succesful) {
        emit transferComplete();
    } else {
        emit transferFailed();
    }
    deleteLater();
}

void FileTransferTask::slotTransferAccepted(Kopete::Transfer*transfer, const QString &fileName) {

    mrimDebug() << fileName;
    d->tranfserTask = transfer; //->info().saveToDirectory();

    d->fileNames.append(fileName);

    connect(this, SIGNAL(bytesSent(uint)),
            d->tranfserTask, SLOT(slotProcessed(uint)));

    connect(this, SIGNAL(transferComplete()),
            d->tranfserTask, SLOT(slotComplete()));

    openSocket(&d->transferInfo);
}

void FileTransferTask::slotTransferRefused(const Kopete::FileTransferInfo &fileTransferInfo) {
    /// @todo cancel transfer
    deleteLater();
}
