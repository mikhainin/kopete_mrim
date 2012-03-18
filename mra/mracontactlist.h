#ifndef MRACONTACTLIST_H_
#define MRACONTACTLIST_H_

#include "mracontactlistentry.h"
#include <QString>
#include <QVector>

struct  MRAGroup {
	ulong flags;
	QString name;
};


class MRAGroups {
private:
	QVector <MRAGroup>m_groups;
public:
	void add(MRAGroup& newGroup){ 
        m_groups.push_back(newGroup); 
    }
	
	int count() const { 
        return m_groups.size(); 
    }
	
	const MRAGroup& operator [] (int index) const { 
        return m_groups[index]; 
    }

	
};

class MRAContactList
{
	std::vector<MRAContactListEntry> m_items;
	MRAGroups m_groups;
    int m_status;
public:
	MRAContactList();
	~MRAContactList();
	
	const MRAContactListEntry& operator [] (int index) const;
	
	void addEntry(const MRAContactListEntry& newEntry);
	int count() const; 
    
	const MRAGroups & groups() const {
         return m_groups;
    }
    
    MRAGroups & groups() {
         return m_groups;
    }
    
    void setStatus(int status);
	int status() const;
};

#endif /*MRACONTACTLIST_H_*/
