#ifndef MRIMCLIENT_H
#define MRIMCLIENT_H

#include <QObject>

class MrimClient : public QObject
{
    Q_OBJECT
public:
    explicit MrimClient(QObject *parent = 0);
    
    void connect(const QString &username, const QString &password);
signals:
    
public slots:
    
private:
    
};

#endif // MRIMCLIENT_H
