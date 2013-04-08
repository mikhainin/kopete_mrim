#ifndef QTMRA_TRANSFERMANAGER_H
#define QTMRA_TRANSFERMANAGER_H

class QString;

namespace qtmra {
class IFileTransferInfo;

class TransferManager
{
public:
    TransferManager();
    ~TransferManager();

    bool hasSession(const QString &remoteUser, int sessionId);

    void addSession(IFileTransferInfo *info);
    void removeSession(const QString &remoteUser, int sessionId);

    IFileTransferInfo *session(const QString &remoteUser, int sessionId);
private:
    class Private;
    Private *d;
};

} // namespace qtmra

#endif // QTMRA_TRANSFERMANAGER_H
