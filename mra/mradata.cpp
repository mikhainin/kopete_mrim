#include <QtCore>
#include <string>
#include "mradata.h"


class CodecHolder {
public:
    CodecHolder(QByteArray name): m_oldCodec( QTextCodec::codecForCStrings() ) {
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName(name));
    }
    ~CodecHolder() {
        QTextCodec::setCodecForCStrings(m_oldCodec);
    }
private:
    QTextCodec *m_oldCodec;
};



MRAData::MRAData(QObject * parent)
    : QObject(parent)
    , m_pointer(0)
{
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
    CodecHolder holder("Windows-1251");

    QByteArray ba = str.toAscii();

     addInt32(ba.size());
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
    addInt32(data.size());
    addData(data);
}

/*!
    \fn MRAData::addInt32(long int value)
 */
void MRAData::addInt32(quint32 value)
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
quint32 MRAData::getInt32()
{
    quint32 result = 0;

    if (m_pointer <= (getSize() - static_cast<int>(sizeof result)) ) {

        result     = *(quint32*)(getData() + m_pointer) ;

        std::cout << std::hex << static_cast< unsigned int >( *(getData() + m_pointer) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+1) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+2) )<< " " <<
                     static_cast< unsigned int >( *(getData() + m_pointer+3) ) << " = " << std::dec << result
                     << std::endl;
        m_pointer +=  sizeof(result);
    }
    return result;
}

QString MRAData::getString()
{
    int len = getInt32();

    if (m_data.size() >= (m_pointer + len)) {
        CodecHolder holder("Windows-1251");

        QString result = QString::fromAscii( m_data.mid(m_pointer, len).constData() );

        m_pointer += len;

        return result;
    } else {
        return QString();
    }
}

QString MRAData::getUnicodeString() {
    int len = getInt32();

    if (m_data.size() >= (m_pointer + len)) {
        CodecHolder holder("UTF-16LE");

        QString result = QString::fromAscii( m_data.mid(m_pointer, len).constData(), len );

        m_pointer += len;

        return result;
    } else {
        return QString();
    }
}

QByteArray MRAData::getUIDL() {

    int len = 8;

    if (m_data.size() >= (m_pointer + len)) {

        QByteArray result = m_data.mid(m_pointer, len);

        m_pointer += len;

        return result;
    } else {
        return QByteArray();
    }

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
