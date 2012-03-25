#include <QStringList>

#include "mraofflinemessage.h"

MRAOfflineMessage::MRAOfflineMessage(QObject *parent, const QByteArray &id) :
    QObject(parent)
  , m_id(id)
  , m_flags(0)
  , m_protoVersion(0)
{
}

void MRAOfflineMessage::parse(const QString &rfc822) {
    QString message = rfc822;

    QMap<QString, QString> headers;

    cutHeaders(headers, message);

    m_date      = KDateTime::fromString(headers["Date"], KDateTime::RFCDate);
    m_from      = headers["From"];
    m_subject   = headers["Subject"];


    QStringList version  = headers["Version"].split('.');

    m_protoVersion =
            version[0].toUInt() << 2 |
            version[1].toUInt()      ;

    m_flags = headers["X-MRIM-Flags"].toUInt(0, 16);

    QString boundary    = headers["Boundary"];

    QStringList parts   = message.split("--" + boundary + "--\n");

    m_text    = parts[0].trimmed();
    m_rtfText = parts[1].trimmed();

}

void MRAOfflineMessage::cutHeaders(QMap<QString, QString> &headers, QString &message) {
    int headerEnd = message.indexOf("\n\n");
    QString headerPart = message.left(headerEnd);
    message = message.mid( headerEnd + 2 );

    foreach( const QString &header, headerPart.split("\n") ) {
        int colonPos = header.indexOf(':');

        headers.insert(header.left(colonPos), header.mid(colonPos + 2));
    }
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

#include "mraofflinemessage.moc"
