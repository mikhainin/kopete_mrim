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
#include <kopete/kopeteuiglobal.h>

#include "debug.h"
#include "mra/mra_proto.h"
#include "mra/mraprotocolv123.h"
#include "mra/transferrequestinfo.h"
#include "mrimcontact.h"
#include "mrimaccount.h"
#include "filetransfertask.h"

enum State {
    INITIAL,
    HELLO_SENT,
    COMMANDS,
    DATA
};

/**
 * @todo
 * - delete partly downloaded files
 */

struct FileTransferTask::Private {
    MrimAccount *account;
    MRAProtocolV123 *proto;
    MrimContact *contact;
    QTcpServer *server;
    QTcpSocket *socket;
    quint64 currentFileBytesProcessed;
    quint64 bytesProcessed;
    QStringList fileNames;
    Kopete::Transfer *transferTask;
    QFile *file;
    FileTransferTask::Direction dir;
    TransferRequestInfo transferInfo;
    int sessionId;

    QList<QPair<QString, quint64> > files;

    int currentFile;
    State state;

    Private()
        : account(0)
        , proto(0)
        , contact(0)
        , server(0)
        , socket(0)
        , currentFileBytesProcessed(0)
        , bytesProcessed(0)
        , transferTask(0)
        , file(0)
        , sessionId(0)
        , currentFile(-1)
        , state(INITIAL) {
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

        foreach (const QString &filename, fileNames) {
            QFileInfo finfo(filename);
            QPair<QString, quint64> item;
            item.first = finfo.fileName();
            item.second = finfo.size();

            d->files.append( item );
        }


        d->file = new QFile(fileNames[0], this);

        d->transferTask = Kopete::TransferManager::transferManager()
                ->addTransfer(contact, fileNames, getFilesSize(), contact->contactId(), Kopete::FileTransferInfo::Outgoing);


        connect(this, SIGNAL(bytesSent(uint)),
                d->transferTask, SLOT(slotProcessed(uint)));

        connect(this, SIGNAL(transferComplete()),
                d->transferTask, SLOT(slotComplete()));

        connect(this, SIGNAL(transferFailed()),
                d->transferTask, SLOT(slotCancelled()));

        connect(d->transferTask, SIGNAL(transferCanceled()),
                this, SLOT(slotCancel()) );

        openServer();

        d->proto->startFileTransfer(this);

    } else {

        connect ( Kopete::TransferManager::transferManager(), SIGNAL (accepted(Kopete::Transfer*,QString)),
                  this, SLOT(slotTransferAccepted(Kopete::Transfer*,QString)) );

        connect ( Kopete::TransferManager::transferManager(), SIGNAL (refused(Kopete::FileTransferInfo)),
                  this, SLOT(slotTransferRefused(Kopete::FileTransferInfo)) );

        d->transferInfo = *info;

        d->sessionId = d->transferInfo.sessionId();

        mrimDebug() << "total size: " << d->transferInfo.totalSize();
        Kopete::TransferManager::transferManager()
                ->askIncomingTransfer(contact, d->transferInfo.getFilesAsStringList(), d->transferInfo.totalSize());


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

}

void FileTransferTask::openSocket(const TransferRequestInfo *info) {

    d->socket = new QTcpSocket(this);

    typedef QPair<QString, int> connect_t;
    connect_t connectData;

    foreach (const connect_t &i, info->getHostsAndPorts()) {
        if (i.second != 443) { // SSL is not supported yet
            connectData = i;
            break;
        }
    }

    d->files = info->getFiles();

    d->socket->connectToHost(connectData.first, connectData.second);
    if ( not d->socket->waitForConnected(1000) ) {

        delete d->socket;
        d->socket = 0;
        if (d->dir == Incoming) {
            openServer();
            d->proto->sendTryThisHost(this);
        } else {
            d->proto->sendTransferCantLocal(this);
        }
        return;
    }
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));

    sendHello();
}

void FileTransferTask::tryThisHost(const QString &hosts) {

    QList<QPair<QString, int> > list =
                TransferRequestInfo::parseHostsAndPorts(hosts);


    typedef QPair<QString, int> connect_t;
    connect_t connectData;

    foreach (const connect_t &i, list) {
        if (i.second != 443) { // SSL is not supported yet
            connectData = i;
            break;
        }
    }

    d->socket = new QTcpSocket(this);
    d->socket->connectToHost(connectData.first, connectData.second);
    if ( not d->socket->waitForConnected(1000) ) {
        delete d->socket;
        d->proto->sendTransferCantLocal(this);
        return;
    }
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));

    sendHello();

}

void FileTransferTask::useThisProxy(const QString &hosts, const QByteArray &proxyKey) {
    QList<QPair<QString, int> > list =
                TransferRequestInfo::parseHostsAndPorts(hosts);


    typedef QPair<QString, int> connect_t;
    connect_t connectData;

    foreach (const connect_t &i, list) {
        if (i.second != 443) { // SSL is not supported yet
            connectData = i;
            break;
        }
    }

    d->socket = new QTcpSocket(this);
    d->socket->connectToHost(connectData.first, connectData.second);
    if ( not d->socket->waitForConnected(1000) ) {
        cancel();
        return;

    }

    proxyNegotiate(proxyKey);

    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));

    if (d->dir == Incoming) {
        sendHello();
    }
}

void FileTransferTask::proxyNegotiate(const QByteArray &proxyKey) {

    mrim_packet_header_t header;
    memset(&header, '\0', sizeof header);

    header.dlen = 16;
    header.from = 0;
    header.fromport = 0;
    header.magic = CS_MAGIC;
    header.msg = MRIM_CS_TRANSFER_PROXY_START_SESSION;
    header.proto = PROTO_VERSION;
    header.seq = 0;

    d->socket->write((const char*)&header, sizeof header);
    d->socket->write(proxyKey);
    d->socket->waitForBytesWritten(-1);

    QApplication::processEvents();

    d->socket->waitForReadyRead(-1);

    d->socket->read((char*)&header, sizeof header);

    Q_ASSERT(header.msg == MRIM_CS_TRANSFER_PROXY_START_SESSION_ACK);
}

void FileTransferTask::sendHello() {
    QByteArray hello;
    hello.append("MRA_FT_HELLO ");
    hello.append(d->account->accountId());
    hello.append('\0');

    if (d->socket->write(hello) == -1) {
        /// @todo report error
        mrimWarning() << "error" << d->socket->errorString();
    }
}

QString FileTransferTask::getContact() {
    return d->contact->contactId();
}

QString FileTransferTask::getAccountId() {
    return d->account->accountId();
}

QList<QPair<QString, quint64> > FileTransferTask::getFiles() {
    return d->files;
}

quint64 FileTransferTask::getFilesSize() {
    quint64 sumSizes = 0;
    foreach(const QString &fileName, d->fileNames) {
        sumSizes += QFileInfo(fileName).size();
    }

    return sumSizes;
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
    mrimDebug() << "new connection" ;

    d->socket = d->server->nextPendingConnection();
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotIncommingData()));
    QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

void FileTransferTask::discardClient() {

    if (d->socket->bytesAvailable() > 0) {
        slotIncommingData();
        mrimDebug() << "bytes available" << d->socket->bytesAvailable();
    }
    d->socket->deleteLater();
    d->socket = 0;
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

void FileTransferTask::slotTransferAccepted(Kopete::Transfer*transfer, const QString &filePath) {

    d->proto->addTransferSession(this);

    if (transfer->info().saveToDirectory()) {
        mrimDebug() << "dir";

        foreach (const QString &filename, d->transferInfo.getFilesAsStringList()) {
            d->fileNames.append(filePath + '/' + filename);
            mrimDebug() << "append" << filePath + '/' + filename;
        }
    } else {
        mrimDebug() << "one file" << filePath;
        // only one file
        /// @todo user can change the name of the file
        d->fileNames.append(filePath);
    }
    d->transferTask = transfer;


    connect(this, SIGNAL(bytesSent(uint)),
            d->transferTask, SLOT(slotProcessed(uint)));

    connect(this, SIGNAL(transferComplete()),
            d->transferTask, SLOT(slotComplete()));

    connect(this, SIGNAL(transferFailed()),
            d->transferTask, SLOT(slotCancelled()));

    connect(d->transferTask, SIGNAL(transferCanceled()),
            this, SLOT(slotCancel()) );

    openSocket(&d->transferInfo);

    d->file = new QFile(filePath, this);
    if (not d->file->open(QFile::WriteOnly)) {
        /// @todo report error
        mrimWarning() << "error" << d->file->errorString();
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
        mrimDebug() << "hello";
        commandHello();

    } else if (data.startsWith(getFile)) {

        QString filename = data.mid(getFile.length(), data.size() - getFile.length() - 1); // 1 - zerro-terminator
        mrimDebug() << "get file" << filename;

        commandGetFile(filename);

    } else {
        mrimDebug() << "data" << data.size();
        dataReceived(data);
    }
}


QString FileTransferTask::getFirstFilename() {
    d->currentFile = 0;
    return d->files[d->currentFile].first;
}

QString FileTransferTask::getNextFileName() {
    d->currentFileBytesProcessed = 0;
    d->currentFile++;
    if (d->currentFile >= d->files.length()) {
        return QString();
    }
    return d->files[d->currentFile].first;
}

void FileTransferTask::nextFile(const QString &filename) {

    delete d->file; // it's okay to delete null-pointer
    d->file = 0;

    if (filename.isEmpty()) {
        mrimDebug() << "empty filename";
        finishTransfer(true);
        return;
    }

    if (d->dir == Incoming) {
        QByteArray getFile;
        getFile.append("MRA_FT_GET_FILE ");
        getFile.append(filename);
        getFile.append('\0');

        if (d->socket->write(getFile) == -1) {
            mrimWarning() << "error" << d->socket->errorString();
        }

        d->file = new QFile( filePathByFilename(filename) );
        d->file->open(QFile::WriteOnly);
        d->transferTask->slotNextFile(filename, filePathByFilename(filename));
    } else {

        d->file = new QFile(filePathByFilename(filename), this);
        if ( !d->file->open(QIODevice::ReadOnly) ) {
            /// @todo report error
            mrimDebug() << "shit";
            return;
        }
        d->transferTask->slotNextFile(filename, filePathByFilename(filename));
    }

}

QString FileTransferTask::filePathByFilename(const QString &filename) {
    foreach(const QString &filepath, d->fileNames) {
        if (QFileInfo(filepath).fileName() == filename) {
            return filepath;
        }
    }
    mrimWarning() << "file not found" << filename;
    return QString();
    /// @todo throw
}

void FileTransferTask::commandHello() {
    if (d->dir == Incoming) {

        nextFile(getFirstFilename());

    } else {
        sendHello();
    }
}

void FileTransferTask::commandGetFile(const QString &filename) {

    mrimWarning() << "Get file";

    nextFile(filename);

    connect(d->socket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(slotBytesProcessed(qint64)) );

    d->currentFileBytesProcessed = 0;
    for(int i = 0; i < d->fileNames.size(); ++i) {
        if (QFileInfo(d->fileNames[i]).fileName() == filename) {
            d->currentFile = i;
            break;
        }
    }

    qint64 written = d->socket->write( d->file->read( 5 * 1024 * 1024 ) );
    if ( written == -1 ) {
        /// @todo report error
        mrimWarning() << "error" << d->socket->errorString();
    }
    // slotBytesProcessed(written);
}


void FileTransferTask::slotBytesProcessed(qint64 bytes) {
    QTcpSocket* socket = (QTcpSocket*)sender();

    d->bytesProcessed += bytes;
    d->currentFileBytesProcessed += bytes;

    if (socket->bytesToWrite() == 0 && d->currentFileBytesProcessed < d->files[d->currentFile].second) {
        if (socket->bytesToWrite() == 0) {
            qint64 written = d->socket->write( d->file->read( 5 * 1024 * 1024 ) );
            if (written == -1) {
                 /// @todo report error
                 mrimWarning() << "error" << d->socket->errorString();
            }
            // slotBytesProcessed(written);
        }
    } else if (socket->bytesToWrite() == 0) {

        QObject::disconnect(d->socket, SIGNAL(bytesWritten(qint64)),
                            this, SLOT(slotBytesProcessed(qint64)) );

        if (d->bytesProcessed == getFilesSize()) {
            finishTransfer(true);
        }
    }
    mrimDebug() << "written" << d->bytesProcessed;
    emit bytesSent(d->bytesProcessed);

}


void FileTransferTask::dataReceived(QByteArray &data) {
    d->file->write(data);
    d->file->flush();
    d->bytesProcessed += data.size();
    d->currentFileBytesProcessed += data.size();
    d->transferTask->slotProcessed(d->bytesProcessed);

    if (d->currentFileBytesProcessed == d->files[d->currentFile].second) {
        mrimDebug() << "downloaded";
        while(d->file->bytesToWrite() > 0) {
            mrimDebug() << "bytesToWrite" << d->file->bytesToWrite();
            d->file->waitForBytesWritten(100);
            QApplication::processEvents();
        }
        d->file->flush();

        nextFile(getNextFileName());
        // finishTransfer(true);
    }
}

void FileTransferTask::cancel() {
    finishTransfer(false);
}

void FileTransferTask::slotCancel() {
    cancel();
}
