#include <kdebug.h>
#include <QStringList>
#include <QTextCodec>

#include "mra_proto.h"
#include "mraofflinemessage.h"



class MessagePart {
public:

    MessagePart(const QString &str);

    QString contentType();
    QString boundary();
    QString charset();
    const QString &text() const;
    QString header(const QString &name);
    bool hasHeader(const QString &name) const;

private:
    QMap<QString, QString> m_headers;
    QString m_text;

    void cutHeaders(QMap<QString, QString> &headers, QString &message);
};

MessagePart::MessagePart(const QString &str) : m_text(str.trimmed()) {
    cutHeaders(m_headers, m_text);
}

void MessagePart::cutHeaders(QMap<QString, QString> &headers, QString &message) {

    int start = 0;
    int index = 0;
    while((index = message.indexOf('\n', start)) != -1) {
        const QString &header = message.mid(start, index - start).trimmed();

        if ( header.length() == 0 ) {
            break;
        }

        int colonPos = header.indexOf(':');

        headers.insert(header.left(colonPos), header.mid(colonPos + 2));

        start = index + 1;
    }


    message = message.mid( start ).trimmed();

}

bool MessagePart::hasHeader(const QString &name) const {
     return m_headers.contains(name);
}

QString MessagePart::header(const QString &name) {
     return m_headers[name];
}

const QString &MessagePart::text() const {
    return m_text;
}

QString MessagePart::contentType() {
    const QString &contentType = m_headers["Content-Type"];
    return contentType.left( contentType.indexOf(';') );
}

QString MessagePart::boundary() {

    if (hasHeader("Boundary")) {
        return header("Boundary");
    }

    const QString &contentType = m_headers["Content-Type"];
    return contentType.mid(contentType.indexOf("boundary=") + 9).trimmed();
}

QString MessagePart::charset() {
    const QString &contentType = m_headers["Content-Type"];
    return contentType.mid(contentType.indexOf("charset=") + 8).trimmed();
}


///////////////////////////////////////////////////////////////////////////

MRAOfflineMessage::MRAOfflineMessage(QObject *parent, const QByteArray &id) :
    QObject(parent)
  , m_id(id)
  , m_flags(0)
  , m_protoVersion(0)
{
}

#include <iostream>
void MRAOfflineMessage::parse(const QString &rfc822) {
    MessagePart message(rfc822);

    m_date      = KDateTime::fromString(message.header("Date"), KDateTime::RFCDate);
    m_from      = message.header("From");
    m_subject   = message.header("Subject");

    QStringList version;
    if (message.hasHeader("Version")) {
        version  = message.header("Version").split('.');
    } else {
        version  = message.header("X-MRIM-Version").split('.');
    }

    m_protoVersion = MAKE_VERSION(
                    version[0].toUInt(),
                    version[1].toUInt()
                );

    m_flags = message.header("X-MRIM-Flags").toUInt(0, 16);

    if (message.contentType() == "text/plain") {
        parseTextPart( message );
        return;
    }

    QString boundary = message.boundary();

    QStringList parts   = message.text().split(QRegExp("--" + boundary + "(--)?\r?\n"));

    foreach( const QString &partText, parts ) {
        if (partText.trimmed().length() == 0) {
            continue;
        }

        MessagePart part(partText);
        if (part.contentType() == "text/plain") {
            parseTextPart( part );
        } else {
            m_rtfText = message.text();
        }
    }

}

void MRAOfflineMessage::cutHeaders(QMap<QString, QString> &headers, QString &message) {

    int start = 0;
    int index = 0;
    while((index = message.indexOf('\n', start)) != -1) {
        const QString &header = message.mid(start, index - start).trimmed();

        if ( header.length() == 0 ) {
            break;
        }

        int colonPos = header.indexOf(':');

        headers.insert(header.left(colonPos), header.mid(colonPos + 2));

        start = index + 1;
    }

    message = message.mid( start ).trimmed();

}

const QByteArray &MRAOfflineMessage::id() const {
    return m_id;
}

const KDateTime &MRAOfflineMessage::date() const {
    return m_date;
}

const QString &MRAOfflineMessage::subject() const {
    return m_subject;
}

const QString &MRAOfflineMessage::from() const {
    return m_from;
}

quint32 MRAOfflineMessage::flags() const {
    return m_flags;
}

quint32 MRAOfflineMessage::protoVersion() const {
    return m_protoVersion;
}

const QString &MRAOfflineMessage::text() const {
    return m_text;
}

const QString &MRAOfflineMessage::rtfText() const {
    return m_rtfText;
}

void MRAOfflineMessage::parseTextPart(MessagePart &textPart) {
    if (m_protoVersion < MAKE_VERSION(1,16)) {
        return;
    }
    m_text = textPart.text().trimmed();

    if (textPart.header("Content-Transfer-Encoding") == "base64") {

        QByteArray data;
        data = QByteArray::fromBase64( m_text.toAscii() );
        QTextCodec *codec = QTextCodec::codecForName( textPart.charset().toAscii() );

        m_text = codec->toUnicode(data);
    }
}

#include "mraofflinemessage.moc"
