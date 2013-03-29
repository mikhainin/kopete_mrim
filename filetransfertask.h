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
    int getFileSize();
    int getSessionId();
    QString getHostAndPort();
    QString getAccountId();

signals:
    void transferComplete();
    void transferFailed();
    void bytesSent(uint);



public slots:
    void slotReadOutgoingDataClient();

    void slotReadIncommingDataClient();

    void discardClient();

    void slotIncomingConnection();

    void slotTransferAccepted(Kopete::Transfer*transfer, const QString &fileName);
    void slotTransferRefused(const Kopete::FileTransferInfo &fileTransferInfo);
private:
    class Private;
    Private *d;


    void finishTransfer(bool succesful);
    void openServer();
    void openSocket(const TransferRequestInfo *info);
};

#endif // FILETRANSFERTASK_H
