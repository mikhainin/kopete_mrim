#include <QStringList>
#include <QTextCodec>

#include "mra_proto.h"
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

    QStringList version;
    if (headers.contains("Version")) {
        version  = headers["Version"].split('.');
    } else {
        version  = headers["X-MRIM-Version"].split('.');
    }

    m_protoVersion = MAKE_VERSION(
                    version[0].toUInt(),
                    version[1].toUInt()
                );

    m_flags = headers["X-MRIM-Flags"].toUInt(0, 16);

    QString boundary;
    if (headers.contains("Boundary")) {
        boundary = headers["Boundary"];
    } else {
        QString contentType = headers["Content-Type"];
        boundary = contentType.mid(contentType.indexOf("boundary=") + 9);
    }

    QStringList parts   = message.split(QRegExp("--" + boundary + "(--)?\r?\n"));

    if (parts.size() == 3) {
        m_text    = parts[1].trimmed();
        m_rtfText = parts[2].trimmed();
    } else {
        m_text    = parts[0].trimmed();
        m_rtfText = parts[1].trimmed();
    }

    parseTextPart();

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

void MRAOfflineMessage::parseTextPart() {
    if (m_protoVersion < MAKE_VERSION(1,16)) {
        return;
    }

    QMap<QString, QString> headers;
    cutHeaders(headers, m_text);
    if (headers["Content-Transfer-Encoding"] == "base64") {
        // TODO get character from header
        QByteArray data;
        data = QByteArray::fromBase64(m_text.trimmed().toAscii());
        QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");

        m_text = codec->toUnicode(data);
    }
}

#include "mraofflinemessage.moc"
