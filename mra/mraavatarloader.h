#ifndef MRAAVATARLOADER_H
#define MRAAVATARLOADER_H

#include <QObject>

class MRAAvatarLoaderPrivate;

class QImage;

class MRAAvatarLoader : public QObject
{
    Q_OBJECT
public:
    explicit MRAAvatarLoader(const QString &contact, QObject *parent = 0, bool large = false, QObject *reveiver = 0, const char *member = 0);
    ~MRAAvatarLoader();

    const QImage &image();
    const QString &contact();
    void run();

signals:
    void done(bool success, MRAAvatarLoader *self);

public slots:

private slots:
    void slotHttpHeadDone(bool status);
    void slotHttpHeadHeadersReceived(const QHttpResponseHeader & resp );
    void slotHttpGetHeadersReceived(const QHttpResponseHeader & resp );
    void slotHttpGetRequestFinished(int id, bool error);
private:
    MRAAvatarLoaderPrivate *d;
};

#endif // MRAAVATARLOADER_H
