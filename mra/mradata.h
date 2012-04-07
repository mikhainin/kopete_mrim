#ifndef MRADATA_H
#define MRADATA_H

#include <cstdlib>
#include <iostream>
#include <assert.h>
#include <QObject>
#include <QByteArray>

class MRAData : public QObject {
    Q_OBJECT
public:
    MRAData(QObject * parent = 0);

    ~MRAData();
    void addString(const QString &str);
    QString getString();
    QByteArray getUIDL();

    void addInt32(quint32 value);
    quint32 getInt32();

    const char *getData();
    int getSize() const;
    void addData(const void *data_, ssize_t size);
    void addUIDL(const QByteArray &str);

    bool eof() const;

    void clear();
    void dumpData();
private:
private:
    QByteArray m_data;
    int m_pointer;
};

#endif
