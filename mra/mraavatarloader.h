#ifndef MRAAVATARLOADER_H
#define MRAAVATARLOADER_H

#include <QObject>

class MRAAvatarLoaderPrivate;

class QImage;
class QNetworkReply;

class MRAAvatarLoader : public QObject
{
    Q_OBJECT
public:
    explicit MRAAvatarLoader(const QString &contact, QObject *parent = 0, bool large = false, QObject *receiver = 0, const char *member = 0);
    ~MRAAvatarLoader();

    void run();

    const QImage &image() const;
    const QString &contact() const;

    bool large() const;
    QObject *receiver() const;
    const char *member() const;
signals:
    void done(bool success, MRAAvatarLoader *self);

public slots:

private slots:
    // void slotHttpHeadDone(bool status);
    // void slotHttpHeadHeadersReceived(const QHttpResponseHeader & resp );
    // void slotHttpGetHeadersReceived(const QHttpResponseHeader & resp );
    // void slotHttpGetRequestFinished(int id, bool error);
    void replyFinished(QNetworkReply* reply);
private:
    MRAAvatarLoaderPrivate *d;
};

#endif // MRAAVATARLOADER_H
