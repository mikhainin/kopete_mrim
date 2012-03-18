#include "mracontactlist.h"

MRAContactList::MRAContactList()
{
}

MRAContactList::~MRAContactList()
{
}

const MRAContactListEntry& MRAContactList::operator [] (int index) const
{
	return m_items[index];
}

int MRAContactList::count() const
{
	return m_items.size();
}

void MRAContactList::addEntry(const MRAContactListEntry& newEntry)
{
	m_items.push_back(newEntry);
}

void MRAContactList::setStatus(int status) {
    m_status = status;
}

int MRAContactList::status() const {
    return m_status;
}
