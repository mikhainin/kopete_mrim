#include <sys/types.h>
#include <unistd.h>

#include <QApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <kdebug.h>
#include <kopete/kopetetransfermanager.h>

#include "debug.h"
#include "mra/mraprotocolv123.h"
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
    QString contact;
    QTcpServer sock;
    int bytesSent;
    QStringList fileNames;
    Kopete::Transfer *tranfserTask;
    QFile *file;

    Private()
        : account(0)
        , proto(0)
        , bytesSent(0)
        , tranfserTask(0) {
    }
};

FileTransferTask::FileTransferTask(
        MrimAccount *account,
        const QString &contact,
        QStringList fileNames,
        QObject *parent)
    : QObject(parent)
    , d(new Private)

{
    d->account = account;
    d->contact = contact;
    d->fileNames = fileNames;

    connect(&d->sock, SIGNAL(newConnection()),
            this, SLOT(slotIncomingConnection()) );


    d->sock.listen(QHostAddress("192.168.1.43"), 2041);

    d->proto = dynamic_cast<MRAProtocolV123 *>(account->getMraProtocol());


    d->file = new QFile(fileNames[0], this);

    d->tranfserTask = Kopete::TransferManager::transferManager()
            ->addTransfer((MrimContact*)account->contacts()[contact], fileNames[0], d->file->size(), contact, Kopete::FileTransferInfo::Outgoing);
    connect(this, SIGNAL(bytesSent(uint)),
            d->tranfserTask, SLOT(slotProcessed(uint)));

    connect(this, SIGNAL(transferComplete()),
            d->tranfserTask, SLOT(slotComplete()));


    kDebug(kdeDebugArea()) << "starting";

    QApplication::processEvents();

    d->proto->startFileTransfer(this);
}

FileTransferTask::~FileTransferTask() {
    delete d;
}

QString FileTransferTask::getContact() {
    return d->contact;
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
    return "192.168.1.43:2041;";
}

void FileTransferTask::slotIncomingConnection() {
    kDebug(kdeDebugArea()) << "new connection" ;
    QTcpSocket* s = d->sock.nextPendingConnection();
    QObject::connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    QObject::connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

void FileTransferTask::discardClient() {

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void FileTransferTask::readClient()
{
    // MRA_FT_HELLO mikhail.galanin@bk.ru.
    const QString hello = "MRA_FT_HELLO " + d->contact + QChar('\0');
    // const QString get   = "MRA_FT_GET_FILE ";

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

        connect(socket, SIGNAL(bytesWritten(qint64)),
                d->tranfserTask, SLOT(slotProcessed(uint)));

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
                emit bytesSent(bytesWritten);
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
    d->sock.close();
    if (succesful) {
        emit transferComplete();
    } else {
        emit transferFailed();
    }
    deleteLater();
}
