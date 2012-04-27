#include "mracontactlistentry.h"



MRAContactListEntry::MRAContactListEntry(ulong id)
    : _id(id)
    , _flags(0)
    , _group(0)
    , _server_flags(0)
    , _status(0)
{
}

MRAContactListEntry::MRAContactListEntry()
    : _id(-1)
    , _flags(0)
    , _group(0)
    , _server_flags(0)
    , _status(0) {

}

MRAContactListEntry::~MRAContactListEntry()
{
}

int MRAContactListEntry::id() const {
    return _id;
}

void MRAContactListEntry::setFlags(const ulong flags) {
    _flags = flags;
}

ulong MRAContactListEntry::flags() const {
    return _flags;
}

ulong MRAContactListEntry::group() const {
    return _group;
}

void MRAContactListEntry::setGroup(const ulong group) {
    _group = group;
}

const QString& MRAContactListEntry::address() const {
    return _address;
}

void MRAContactListEntry::setAddress(const QString &address) {
    _address = address;
}

const QString& MRAContactListEntry::nick() const {
    return _nick;
}

void MRAContactListEntry::setNick(const QString &nick) {
    _nick = nick;
}

ulong MRAContactListEntry::serverFlags() const {
    return _server_flags;
}

void MRAContactListEntry::setServerFlags(const ulong sflags) {
    _server_flags = sflags;
}

ulong MRAContactListEntry::status() const {
    return _status;
}

void MRAContactListEntry::setStatus(const ulong status) {
    _status = status;
}
