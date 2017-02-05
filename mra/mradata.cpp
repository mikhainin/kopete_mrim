#include <QtCore>
#include <QTextCodec>
#include <string>
#include "mradata.h"

MRAData::MRAData(QObject * parent)
    : QObject(parent)
    , m_pointer(0)
{
}

MRAData::MRAData(const QByteArray &data, QObject * parent)
    : QObject(parent)
    , m_data(data)
    , m_pointer(0) {

}

MRAData::~MRAData()
{
}

void MRAData::clear()
{
    m_data.clear();
    m_pointer  = 0;

}

/*!
    \fn MRAData::addString()
 */
void MRAData::addString(const QString &str)
{
    QTextCodec *w1251Codec = QTextCodec::codecForName("Windows-1251");
    QByteArray ba = w1251Codec->fromUnicode(str);

     addUint32(ba.size());
     addData(ba.constData(), ba.size());
}

void MRAData::addUIDL(const QByteArray &str)
{
    // UIDL this is a string of 8 bytes
    QByteArray ba = str.left(8);

    addData(ba.constData(), ba.size());
}


/*!
    \fn MRAData::addData(ubsigned char *data, unsigned int size)
 */
void MRAData::addData(const void *data_, ssize_t size)
{
    m_data.append(static_cast<const char*>(data_), size);
}

void MRAData::addData(const QByteArray &data) {
    m_data.append(data);
}

void MRAData::addBinaryString(const QByteArray &data) {
    addUint32(data.size());
    addData(data);
}

QByteArray MRAData::getBinaryString() {
    int len = getUint32();

    if (m_data.size() >= (m_pointer + len)) {

        if (len == 0) {
            return QByteArray();
        }

        QByteArray result = m_data.mid(m_pointer, len);

        m_pointer += len;

        return result;
    } else {
        return QByteArray();
    }
}

/*!
    \fn MRAData::addInt32(long int value)
 */
void MRAData::addUint32(quint32 value)
{
      addData(&value, sizeof(value));
}


/*!
    \fn MRAData::getData()
 */
const char *MRAData::getData()
{
    return m_data.constData();
}


/*!
    \fn MRAData::getSize()
 */
int MRAData::getSize() const
{
    return m_data.size();
}


/*!
    \fn MRAData::getInt32()
 */
quint32 MRAData::getUint32()
{
    quint32 result = 0;

    if (m_pointer <= (getSize() - static_cast<int>(sizeof result)) ) {

        result     = *(quint32*)(getData() + m_pointer) ;
/*
        std::cout << std::hex << static_cast< unsigned int >( *(getData() + m_pointer) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+1) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+2) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+3) ) << " = " << std::dec << result
                     << std::endl; */
        m_pointer +=  sizeof(result);
    }
    return result;
}

QString MRAData::getString()
{
    int len = getUint32();

    if (m_data.size() >= (m_pointer + len)) {

        if (len == 0) {
            return QString();
        }

        // CodecHolder holder("Windows-1251");
        QTextCodec *w1251Codec = QTextCodec::codecForName("Windows-1251");
        QString result = w1251Codec->toUnicode(m_data.mid(m_pointer, len).constData());

        m_pointer += len;

        return result;
    } else {
        return QString();
    }
}

void MRAData::addUnicodeString(const QString &str) {

    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");

    QByteArray ba = codec->fromUnicode(str);

    ba = ba.remove(0, 2); // remove BOM (Byte Order Mark)

    addUint32(ba.size());
    addData(ba.constData(), ba.size());

}

QString MRAData::getUnicodeString() {
    int len = getUint32();

    if (m_data.size() >= (m_pointer + len)) {

        if (len == 0) {
            return QString();
        }

        //CodecHolder holder("UTF-16LE");
        QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");

        QString result = codec->toUnicode( m_data.mid(m_pointer, len).constData(), len );

        m_pointer += len;

        return result;
    } else {
        return QString();
    }
}

QByteArray MRAData::getUIDL() {

    return getNBytes(8);

}

QByteArray MRAData::getNBytes(int n) {

    if (m_data.size() >= (m_pointer + n)) {

        QByteArray result = m_data.mid(m_pointer, n);

        m_pointer += n;

        return result;
    } else {
        return QByteArray();
    }

}

void MRAData::addNBytes(int n, const QByteArray &data) {
    addData(data.constData(), n);
}

QString MRAData::toBase64() const {
    return m_data.toBase64();
}

const QByteArray &MRAData::toByteArray() const {
    return m_data;
}

bool MRAData::eof() const
{
    return (m_pointer >= getSize());
}

void MRAData::dumpData() {
    for (qint32 i = 0; i < m_data.size(); ++i)
    {
        if ( (i % 16) == 0 ) {
            printf("\n");
        }
        printf( "%02x ", unsigned(*(m_data.data() + i)) & 0xFF );
    }
}

#include "mradata.moc"
