#ifndef MRACONTACTLISTENTRY_H_
#define MRACONTACTLISTENTRY_H_

#include <QString>

class MRAContactListEntry
{

private:
    // uussuu (flags, group, address, nick, server flags, current status)

public:
    MRAContactListEntry(ulong id);
    MRAContactListEntry();

    ~MRAContactListEntry();

    ulong flags() const;
    void setFlags(const ulong flags);

    ulong group() const;
    void setGroup(const ulong group);

    const QString& address() const;
    void setAddress(const QString &address);

    const QString& nick() const;
    void setNick(const QString &nick);

    ulong status() const;
    void setStatus(const ulong status);

    ulong serverFlags() const;
    void setServerFlags(const ulong sflags);

    int id() const;

private:
    int _id;
    ulong _flags;
    ulong _group;
    QString _address;
    QString _nick;
    ulong _server_flags;
    ulong _status;
};

#endif /*MRACONTACTLISTENTRY_H_*/
