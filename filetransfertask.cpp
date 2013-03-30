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
    COMMANDS,
    DATA

};

/**
 * @todo
 * - cancel transfer by user (via tray)
 * - try to open channel to remote client when it asks
 * - send/receive many files
 */

struct FileTransferTask::Private {
    MrimAccount *account;
    MRAProtocolV123 *proto;
    MrimContact *contact;
    QTcpServer *server;
    QTcpSocket *socket;
    int bytesProcessed;
    QStringList fileNames;
    Kopete::Transfer *tranfserTask;
    QFile *file;
    FileTransferTask::Direction dir;
    TransferRequestInfo transferInfo;
    int sessionId;

    QList<QPair<QString, int> > files;

    Private()
        : account(0)
        , proto(0)
        , contact(0)
        , server(0)
        , socket(0)
        , bytesProcessed(0)
        , tranfserTask(0)
        , file(0)
        , sessionId(0) {
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

        connect(this, SIGNAL(transferFailed()),
                d->tranfserTask, SLOT(slotCancelled()));

        connect(d->tranfserTask, SIGNAL(transferCanceled()),
                this, SLOT(slotCancel()) );

        openServer();
    } else {

        connect ( Kopete::TransferManager::transferManager(), SIGNAL (accepted(Kopete::Transfer*,QString)),
                  this, SLOT(slotTransferAccepted(Kopete::Transfer*,QString)) );

        connect ( Kopete::TransferManager::transferManager(), SIGNAL (refused(Kopete::FileTransferInfo)),
                  this, SLOT(slotTransferRefused(Kopete::FileTransferInfo)) );

        d->transferInfo = *info;

        d->sessionId = d->transferInfo.sessionId();

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


    d->server->listen(QHostAddress::Any, 2041);

    d->proto->startFileTransfer(this);
}

void FileTransferTask::openSocket(const TransferRequestInfo *info) {
    d->socket = new QTcpSocket(this);
    QPair<QString, int> connectData = info->getHostsAndPorts()[0];
    d->files = info->getFiles();

    d->socket->connectToHost(connectData.first, connectData.second);
    if ( not d->socket->waitForConnected(5000) ) {
        /// @todo report error
    }
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));

    sendHello();
}

void FileTransferTask::sendHello() {
    QByteArray hello;
    hello.append("MRA_FT_HELLO ");
    hello.append(d->account->accountId());
    hello.append('\0');

    if (d->socket->write(hello) == -1) {
        /// @todo report error
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
    if (d->sessionId == 0) {
        d->sessionId = getpid() + i++;
    }
    return d->sessionId;
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

    d->socket = d->server->nextPendingConnection();
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

void FileTransferTask::discardClient() {

    QTcpSocket* socket = (QTcpSocket*)sender();
    if (d->socket->bytesAvailable() > 0) {
        slotIncommingData();
        mrimDebug() << "bytes available" << d->socket->bytesAvailable();
    }
    d->socket->deleteLater();
    d->socket = 0;
}

void FileTransferTask::slotBytesProcessed(qint64 bytes) {
    QTcpSocket* socket = (QTcpSocket*)sender();

    d->bytesProcessed += bytes;
    emit bytesSent(d->bytesProcessed);
    if (socket->bytesToWrite() == 0) {
        finishTransfer(true);
    }
}

void FileTransferTask::finishTransfer(bool succesful) {
    mrimDebug() << "done";
    if (d->server) {
        d->server->close();
    } else if (d->socket) {
        d->socket->close();
    }
    if (succesful) {
        emit transferComplete();
        d->proto->finishFileTransfer(this);
    } else {
        emit transferFailed();
        d->proto->cancelFileTransfer(this);
    }
    deleteLater();
}

void FileTransferTask::slotTransferAccepted(Kopete::Transfer*transfer, const QString &fileName) {

    mrimDebug() << fileName;
    d->tranfserTask = transfer;

    d->fileNames.append(fileName);

    connect(this, SIGNAL(bytesSent(uint)),
            d->tranfserTask, SLOT(slotProcessed(uint)));

    connect(this, SIGNAL(transferComplete()),
            d->tranfserTask, SLOT(slotComplete()));

    connect(this, SIGNAL(transferFailed()),
            d->tranfserTask, SLOT(slotCancelled()));

    connect(d->tranfserTask, SIGNAL(transferCanceled()),
            this, SLOT(slotCancel()) );

    openSocket(&d->transferInfo);

    d->file = new QFile(fileName, this);
    if (not d->file->open(QFile::WriteOnly)) {
        /// @todo report error
    }

}

void FileTransferTask::slotTransferRefused(const Kopete::FileTransferInfo &fileTransferInfo) {
    Q_UNUSED(fileTransferInfo);
    cancel();
}

void FileTransferTask::slotIncommingData() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray data = socket->readAll();

    /// @todo wait for zero-terminator
    const QByteArray hello = "MRA_FT_HELLO ";
    const QByteArray getFile = "MRA_FT_GET_FILE ";

    if (data.startsWith(hello)) {
        commandHello();
    } else if (data.startsWith(getFile)) {

        QString filename = data.mid(getFile.length(), data.size() - getFile.length() - 1); // 1 - zerro-terminator

        commandGetFile(filename);

    } else {
        dataReceived(data);
    }
}

void FileTransferTask::commandHello() {
    if (d->dir == Incoming) {
        QByteArray getFile;
        getFile.append("MRA_FT_GET_FILE ");
        getFile.append(d->files[0].first);
        getFile.append('\0');

        d->socket->write(getFile);
    } else {
        sendHello();
    }
}

void FileTransferTask::commandGetFile(const QString &filename) {

    mrimDebug() << "filename = " << filename;
    d->file = new QFile(d->fileNames[0], this);

    if ( !d->file->open(QIODevice::ReadOnly) ) {
        /// @todo report error
        mrimDebug() << "shit";
        return;
    }

    QByteArray fileData = d->file->readAll();

    connect(d->socket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(slotBytesProcessed(qint64)) );

    if (d->socket->write(fileData) == -1) {
        /// @todo report error
    }

}

void FileTransferTask::dataReceived(QByteArray &data) {
    d->file->write(data);
    d->file->flush();
    d->bytesProcessed += data.size();
    d->tranfserTask->slotProcessed(d->bytesProcessed);

    if (d->bytesProcessed == d->files[0].second) {
        mrimDebug() << "downloaded";
        while(d->file->bytesToWrite() > 0) {
            mrimDebug() << "bytesToWrite" << d->file->bytesToWrite();
            d->file->waitForBytesWritten(100);
            QApplication::processEvents();
        }
        d->file->flush();
        finishTransfer(true);
    }
}

void FileTransferTask::cancel() {
    finishTransfer(false);
}

void FileTransferTask::slotCancel() {
    cancel();
}
