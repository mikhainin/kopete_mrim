#include <kdebug.h>

#include <QImage>
#include <QHttp>


#include "mraavatarloader.h"


struct MRAAvatarLoaderPrivate {
    QString contact;
    QString address;

    QHttp http;
    QImage image;

    int getId;
    QObject *receiver;
    const char *member;
    bool large;

    MRAAvatarLoaderPrivate() : getId(-1), receiver(0), member(0), large(false) {}
};

MRAAvatarLoader::MRAAvatarLoader(const QString &contact, QObject *parent, bool large, QObject *receiver, const char *member) :
    QObject(parent)
{
    d = new MRAAvatarLoaderPrivate;
    d->contact = contact;

    if (receiver && member) {
        QObject::connect(this, SIGNAL(done(bool,MRAAvatarLoader*)), receiver, member);
    }

    d->large = large;

}

MRAAvatarLoader::~MRAAvatarLoader() {
    delete d;
}

void MRAAvatarLoader::run() {


    QStringList items = d->contact.split('@');

    if (items.size() != 2) {
        emit done(false, this);
        return;
    }

    const QString &domain = items[1];

    if (domain == "corp.mail.ru") {
        items[1] = "corp";
    } else {
        items[1] = domain.left(domain.length() - 3); // delete ".ru": bk.ru -> bk
    }
    if (d->large) {
        d->address = "http://obraz.foto.mail.ru/%1/%2/_mrimavatar";
    } else {
        d->address = "http://obraz.foto.mail.ru/%1/%2/_mrimavatarsmall";
    }
    kWarning() << d->contact << d->address ;
    d->address = d->address.arg(items[1], items[0]);

    connect(&d->http, SIGNAL(done(bool)), this, SLOT(slotHttpHeadDone(bool)));

    connect(&d->http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(slotHttpHeadHeadersReceived(QHttpResponseHeader)) );

    d->http.setHost("obraz.foto.mail.ru");

    d->http.head(d->address);

}

const QImage &MRAAvatarLoader::image() const {
    return d->image;
}

const QString &MRAAvatarLoader::contact() const {
    return d->contact;
}

bool MRAAvatarLoader::large() const {
    return d->large;
}

QObject *MRAAvatarLoader::receiver() const {
    return d->receiver;
}

const char *MRAAvatarLoader::member() const {
    return d->member;
}

void MRAAvatarLoader::slotHttpHeadDone(bool error) {
    kWarning() << error << d->http.errorString();

    if (error) {
        emit done(false, this);
        return;
    }

}

void MRAAvatarLoader::slotHttpHeadHeadersReceived(const QHttpResponseHeader & resp ) {

    disconnect(&d->http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
               this, SLOT(slotHttpHeadHeadersReceived(QHttpResponseHeader)) );

    kWarning() << resp.statusCode() << d->contact;

    if (resp.statusCode() == 404) {
        emit done(false, this);
        return;
    }

    connect(&d->http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(slotHttpGetHeadersReceived(QHttpResponseHeader)));

    connect(&d->http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(slotHttpGetRequestFinished(int,bool)) );

    d->getId = d->http.get(d->address);

}

void MRAAvatarLoader::slotHttpGetHeadersReceived(const QHttpResponseHeader & resp ) {

    disconnect(&d->http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)));

    if (resp.statusCode() == 404) {
        emit done(false, this);
        return;
    }

    kWarning() << resp.statusCode() << d->contact;

}

void MRAAvatarLoader::slotHttpGetRequestFinished(int id, bool error) {

    if (id != d->getId) {
        return;
    }

    if (error) {
        emit done(false, this);
        return;
    }

    kWarning() << d->http.bytesAvailable() << d->contact;

    QByteArray data = d->http.readAll();

    d->image.loadFromData(data);

    emit done(true, this);
}

#include "mraavatarloader.moc"
