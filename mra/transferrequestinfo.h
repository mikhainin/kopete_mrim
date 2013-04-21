#ifndef TRANSFERREQUESTINFO_H
#define TRANSFERREQUESTINFO_H

#include <QString>
#include <QStringList>
#include <QPair>
#include <QList>

class TransferRequestInfo
{
public:
    TransferRequestInfo();


    void setRemoteContact(const QString &arg);
    const QString &remoteContact() const;

    void setSessionId(int arg);
    int sessionId() const;

    void setTotalSize(quint64 arg);
    quint64 totalSize() const;

    void setFilesString(const QString &arg);
    QStringList getFilesAsStringList() const;
    QList<QPair<QString, quint64> > getFiles() const;


    void setHostsAndPortsString(const QString &arg) ;
    QList<QPair<QString, int> > getHostsAndPorts() const;
    static QList<QPair<QString, int> > parseHostsAndPorts(const QString &str);
private:
    QString m_remoteContact;
    int m_sessionId;
    quint64 m_totalSize;
    QString m_files;
    QString m_hostsAndPorts;
};

#endif // TRANSFERREQUESTINFO_H
