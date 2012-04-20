#ifndef MRAOFFLINEMESSAGE_H
#define MRAOFFLINEMESSAGE_H

#include <QObject>
#include <QMap>
#include <QString>

#include <kdatetime.h>

class MessagePart;
class MRAOfflineMessage : public QObject
{
    Q_OBJECT
public:
    explicit MRAOfflineMessage(QObject *parent = 0, const QByteArray &id = "");

    void parse(const QString &rfc822);

    const QByteArray &id() const;
    const KDateTime &date() const;
    const QString &subject() const;
    const QString &from() const;
    quint32 flags() const;
    quint32 protoVersion() const;
    const QString &text() const;
    const QString &rtfText() const;

private:
    QByteArray m_id;
    KDateTime m_date;
    QString m_subject;
    QString m_from;
    quint32 m_flags;
    quint32 m_protoVersion;

    QString m_text;
    QString m_rtfText;


    void cutHeaders(QMap<QString, QString> &headers, QString &message);
    void parseTextPart(MessagePart &mainPart, MessagePart &textPart);
signals:

public slots:

};

#endif // MRAOFFLINEMESSAGE_H
