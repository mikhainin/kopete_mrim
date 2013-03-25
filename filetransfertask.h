#ifndef FILETRANSFERTASK_H
#define FILETRANSFERTASK_H

#include <QObject>
#include "mra/mraprotocolv123.h"
class MrimAccount;

class FileTransferTask : public QObject, public IFileTransferInfo
{
    Q_OBJECT
public:
    explicit FileTransferTask(
            MrimAccount *account,
            const QString &contact,
            QStringList fileNames,
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
    void readClient();

    void discardClient();

    void slotIncomingConnection();

private:
    class Private;
    Private *d;


    void finishTransfer(bool succesful);
};

#endif // FILETRANSFERTASK_H
