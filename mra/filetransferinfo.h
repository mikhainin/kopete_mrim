#ifndef FILETRANSFERINFO_H
#define FILETRANSFERINFO_H
#include <QtCore/QPair>
namespace qtmra {

class IFileTransferInfo {
public:
    virtual ~IFileTransferInfo() {}
    virtual QString getContact() = 0;
    virtual QList<QPair<QString, int> > getFiles() = 0;
    virtual int getFilesSize() = 0;
    virtual int getSessionId() = 0;
    virtual QString getHostAndPort() = 0;
    virtual QString getAccountId() = 0;
    virtual void cancel() = 0;
    virtual void tryThisHost(const QString &hots) = 0;
    virtual void useThisProxy(const QString &hots, const QByteArray &proxyKey) = 0;

/**
 * @todo: signals
 */

};

}

#endif // FILETRANSFERINFO_H
