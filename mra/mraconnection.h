#ifndef MRACONNECTION_H
#define MRACONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "mra_proto.h"
#include "mradata.h"


#include <iostream>
#include <cstdlib>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class MRAConnection : public QObject{
    Q_OBJECT
private:


public:
    MRAConnection(QObject *parent = 0);

    class CantEstablishConnection;
    ~MRAConnection();
    bool connectToHost();
    ssize_t write(const char *buf, ssize_t size);
    ssize_t read(char* buf, ssize_t size);

    void sendMsg(mrim_msg_t msg, MRAData *data);
    // void sendMsg(mrim_msg_t msg, int message_id, MRAData *data);

    ssize_t readMessage(mrim_msg_t &msg, MRAData *data);

    void disconnect();
signals:

    void onData();
    void disconnected(const QString &reason);

private slots:
    void slotReadyRead();
    void slotDisconnected();

private:
    mrim_packet_header_t header;

    quint16 m_localPort;
    quint32 m_localAddress;
    quint16 m_remotePort;
    quint32 m_remoteAddress;
    QTcpSocket *m_socket;
    bool m_locked;

    QString getRecommendedServer();
};

#endif
