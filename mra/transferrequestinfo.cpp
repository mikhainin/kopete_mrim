#include <QStringList>

#include "../debug.h"
#include "transferrequestinfo.h"

TransferRequestInfo::TransferRequestInfo()
    : m_sessionId(0)
    , m_totalSize(0)
{
}

void TransferRequestInfo::setRemoteContact(const QString &arg) {
    m_remoteContact = arg;
}

const QString &TransferRequestInfo::remoteContact() const {
    return m_remoteContact;
}

void TransferRequestInfo::setSessionId(int arg) {
    m_sessionId = arg;
}

int TransferRequestInfo::sessionId() const {
    return m_sessionId;
}

void TransferRequestInfo::setTotalSize(quint64 arg) {
    m_totalSize = arg;
}

quint64 TransferRequestInfo::totalSize() const {
    return m_totalSize;
}

void TransferRequestInfo::setFilesString(const QString &arg) {
    m_files = arg;
}

QStringList TransferRequestInfo::getFilesAsStringList() const {

    QStringList result;

    typedef QPair<QString, quint64> list_item;
    foreach(const list_item &item, getFiles()) {
        result.append(item.first);
    }

    return result;

}


QList<QPair<QString, quint64> > TransferRequestInfo::getFiles() const {
    QStringList chunks = m_files.split(';');
    QStringList::const_iterator p = chunks.constBegin();

    QList<QPair<QString, quint64> > res;

    mrimDebug() << m_files;
    for(; p != chunks.constEnd(); ++p) {
        const QString &fileName = *p;
        if (fileName.isEmpty()) {
            break;
        }
        ++p;

        /// @todo check if toInt() was successful
        quint64 fileSize = p->toInt();
        mrimDebug() << fileName << fileSize;
        res.append( QPair<QString, quint64>(fileName, fileSize) );
    }

    return res;

}

void TransferRequestInfo::setHostsAndPortsString(const QString &arg) {
    m_hostsAndPorts = arg;
}

QList<QPair<QString, int> > TransferRequestInfo::getHostsAndPorts() const {
    return parseHostsAndPorts(m_hostsAndPorts);
}

QList<QPair<QString, int> > TransferRequestInfo::parseHostsAndPorts(const QString &str) {
    QStringList chunks = str.split(';');
    QStringList::const_iterator p = chunks.constBegin();

    QList<QPair<QString, int> > res;

    for(; p != chunks.constEnd(); ++p) {
        const QStringList hostAndPort = p->split(':');
        const QString &hostAddress = hostAndPort[0];
        if (hostAddress.isEmpty()) {
            break;
        }

        /// @todo check if toULongLong() was successful
        int port = hostAndPort[1].toULongLong();
        res.append( QPair<QString, int>(hostAddress, port) );
    }

    return res;

}

