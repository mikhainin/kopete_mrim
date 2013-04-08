#include <QtCore/QMap>
#include "filetransferinfo.h"
#include "../debug.h"
#include "transfermanager.h"

namespace qtmra {

struct TransferSessionKey {
    QString remoteUser;
    int sessionId;

    TransferSessionKey(QString remoteUser, int sessionId)
        : remoteUser(remoteUser)
        , sessionId(sessionId) {
    }
    bool operator < (const TransferSessionKey &b) const {
        if (remoteUser != b.remoteUser) {
            return remoteUser < b.remoteUser;
        }
        return sessionId < b.sessionId;
    }
};


struct TransferManager::Private {
    QMap<TransferSessionKey, IFileTransferInfo*> sessions;
};

TransferManager::TransferManager() : d(new Private)
{
}

TransferManager::~TransferManager() {
    delete d;
}

bool TransferManager::hasSession(const QString &remoteUser, int sessionId) {
    return d->sessions.contains(TransferSessionKey(remoteUser, sessionId));
}

void TransferManager::addSession(IFileTransferInfo *info) {
    d->sessions.insert(
                    TransferSessionKey(info->getContact(), info->getSessionId()),
                    info
                );
}

void TransferManager::removeSession(const QString &remoteUser, int sessionId) {
    /// @todo delete session
    mrimDebug() << "removing session " << sessionId << "with" << remoteUser;
    d->sessions.remove(TransferSessionKey(remoteUser, sessionId));
}

IFileTransferInfo *TransferManager::session(const QString &remoteUser, int sessionId) {
    Q_ASSERT(hasSession(remoteUser, session));
    return d->sessions[TransferSessionKey(remoteUser, sessionId)];
}

} // namespace qtmra
