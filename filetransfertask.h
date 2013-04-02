#ifndef FILETRANSFERTASK_H
#define FILETRANSFERTASK_H

#include <QObject>
#include "mra/mraprotocolv123.h"
class MrimAccount;
class MrimContact;

namespace Kopete {
    class Transfer;
    class FileTransferInfo;
}

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QByteArray;
class QString;
QT_END_NAMESPACE

class FileTransferTask : public QObject, public IFileTransferInfo
{
    Q_OBJECT
public:

    enum Direction {
        Incoming,
        Outgoing
    };

    explicit FileTransferTask(MrimAccount *account,
            MrimContact *contact,
            QStringList fileNames,
            Direction dir,
            const TransferRequestInfo *info,
            QObject *parent = 0);

    virtual ~FileTransferTask();

    void run();

    QString getContact();
    QString getFilePath();
    int getFilesSize();
    int getSessionId();
    QString getHostAndPort();
    QString getAccountId();

    void cancel();
signals:
    void transferComplete();
    void transferFailed();
    void bytesSent(uint);



public slots:

    void slotIncommingData();

    void discardClient();

    void slotIncomingConnection();

    void slotTransferAccepted(Kopete::Transfer*transfer, const QString &filePath);
    void slotTransferRefused(const Kopete::FileTransferInfo &fileTransferInfo);
    void slotBytesProcessed(qint64 bytes);
    void slotCancel();
private:
    class Private;
    Private *d;


    void finishTransfer(bool succesful);
    void openServer();
    void openSocket(const TransferRequestInfo *info);
    void sendHello();

    void commandHello();
    void commandGetFile(const QString &filename);
    void dataReceived(QByteArray &data);
    QString getFirstFilename();
    QString getNextFileName();
    QString filePathByFilename(const QString &filename);
    void nextFile(const QString &filename);
};

#endif // FILETRANSFERTASK_H
