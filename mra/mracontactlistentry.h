#ifndef MRACONTACTLISTENTRY_H_
#define MRACONTACTLISTENTRY_H_

#include <QString>

class MRAContactListEntry
{

private:
    // uussuu (флаги, группа, адрес, ник, серверные флаги, текущий статус в сети)
    ulong _flags;
    ulong _group;
    QString _address;
    QString _nick;
    ulong _server_flags;
    ulong _status;

public:
    MRAContactListEntry();
    ~MRAContactListEntry();

    ulong flags() const {return this->_flags;}
    ulong group() const {return this->_group;}
    const QString& address() const {return this->_address;}
    const QString& nick() const {return this->_nick;}
    ulong serverFlags() const {return this->_server_flags;}
    ulong status() const {return this->_status;}

    inline void setFlags(const ulong flags) {
        this->_flags = flags;
    }
    inline void setGroup(const ulong group) {
        this->_group = group;
    }
    inline void setAddress(const QString &address) {
        this->_address = address;
    }
    inline void setNick(const QString &nick) {this->_nick = nick;}
    inline void setServerFlags(const ulong sflags) {this->_server_flags = sflags;}
    inline void setStatus(const ulong status) {this->_status = status;}


};

#endif /*MRACONTACTLISTENTRY_H_*/
