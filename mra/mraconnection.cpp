/***************************************************************************

 *   Copyright (C) 2007 by netgr@am   *
 *   a@localhost   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QHostAddress>
#include <QStringList>

#include <kdebug.h>

#include "mraconnection.h"
#include <arpa/inet.h>


class LockWrapper {
public:
    LockWrapper(bool &lock) : m_lock(lock) {
        m_lock = true;
    }
    ~LockWrapper() {
        m_lock = false;
    }

private:
    bool &m_lock;
};

MRAConnection::MRAConnection(QObject *parent)
    : QObject(parent)
    , m_locked(false)
{
    memset ( &header, 0, sizeof header );

    header.seq = 0;
    header.magic = CS_MAGIC;
    header.proto = PROTO_VERSION;

    header.from = 0;
    header.fromport = 0;

}


MRAConnection::~MRAConnection()
{
    disconnect();
}




/*!
    \fn MRAConnection::connect()
 */
bool MRAConnection::connectToHost()
{
    QString hostAndPort = getRecommendedServer();
    QStringList list = hostAndPort.split(':');

    m_socket = new QTcpSocket( this );

    m_socket->connectToHost(list[0], list[1].toInt());

    if (m_socket->waitForConnected(-1))
        kWarning() << "Connected!";
    else
        kWarning() << m_socket->errorString();

    m_localPort = m_socket->localPort();

    header.fromport = htons(m_localPort);

    m_localAddress = m_socket->localAddress().toIPv4Address();
    header.from =  htonl(m_localAddress);

    connect( m_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
    connect( m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
    return m_socket->isOpen();

}

QString MRAConnection::getRecommendedServer() {
    QTcpSocket socket;

    socket.connectToHost("mrim.mail.ru", 2042);
    socket.waitForConnected(-1); /// @fixme check return status
    socket.waitForReadyRead(-1); /// @fixme check return status
    QByteArray ba = socket.readLine();
    QString hostAndPort(ba);
    hostAndPort = hostAndPort.trimmed();
    kWarning() << "recommended address is " << hostAndPort;

    return hostAndPort;
}

ssize_t MRAConnection::write(const char* buf, ssize_t size)
{
    LockWrapper locker(m_locked);
    ssize_t temp = m_socket->write(buf, size);
    kWarning() << "size: " << size << " written:" << temp;
    return temp;
}

ssize_t MRAConnection::read(char* buf, ssize_t size)
{
    LockWrapper locker(m_locked);

    if (size == 0) {
        return 0;
    }

    ssize_t temp = 0;
    do {

        qint64 read = m_socket->read(buf + temp, size - temp);
        if (read == -1) {
            if ( m_socket->isReadable() )
            kWarning() << "error: " << m_socket->errorString();
            return temp;/// @todo throw!
        } else if (read == 0) {
            m_socket->waitForReadyRead(-1);
        }
        temp += read;

    } while(temp != size);
    kWarning() << "buf:" << size << " read:" << temp;
    return temp;
}


ssize_t MRAConnection::readMessage(mrim_msg_t &msg_, MRAData *data)
{
    mrim_packet_header_t head_	;

    bzero ( &head_ , sizeof head_ );

    ssize_t sz = 0;
    sz = this->read((char*)&head_, sizeof head_ );

    kWarning() << "message: " << head_.msg << " dlen" << head_.dlen;

    msg_ = head_.msg;
    if (sz > 0) {

        QByteArray buf(head_.dlen, 0);

        sz = this->read(buf.data(), head_.dlen);

        if (data && sz > 0) {
            data->addData(buf.constData(), sz);
        }
    }

    if (m_socket->bytesAvailable() > 0) {
        onData();
    }

    return sz;

}


void MRAConnection::sendMsg(mrim_msg_t msg, MRAData *data)
{
    ssize_t sz = 0;
    mrim_packet_header_t currHeader = header;

    currHeader.msg = msg;

    if (data != NULL) {
        currHeader.dlen = data->getSize();
        kWarning() << "dlen: " << currHeader.dlen;
    } else {
        currHeader.dlen = 0;
    }

    sz = this->write((char*)&currHeader, sizeof currHeader );

    if (data != NULL) {
        sz = this->write((char*)(data->getData()), data->getSize());
    }

    std::cout << "written " << sz << " msg: " << msg << " sz: " << sz << " seq: " << currHeader.seq << std::endl;
    std::cout.flush();

    header.seq++;

}


/*!
    \fn MRAConnection::disconnect()
 */
void MRAConnection::disconnect()
{
    if (m_socket) {

        QObject::disconnect(m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );

        m_socket->deleteLater();// isconnectFromHost();
        // delete m_socket;
        m_socket = 0;
    }
}

void MRAConnection::slotReadyRead() {
    if (!m_locked) {
        emit onData();
    }
}

void MRAConnection::slotDisconnected() {
    if (m_socket->errorString().size() > 0 ) {
        emit disconnected( m_socket->errorString() );
    } else {
        emit disconnected("internal error");
    }
}

#include "mraconnection.moc"
