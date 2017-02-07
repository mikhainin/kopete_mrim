#ifndef DEBUG_H
#define DEBUG_H

#include <qdebug.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MRIM_LOG_LISTER)

#define mrimDebug() qDebug(MRIM_LOG_LISTER)

#define mrimWarning() qWarning(MRIM_LOG_LISTER)

#endif // DEBUG_H
