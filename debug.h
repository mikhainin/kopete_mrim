#ifndef DEBUG_H
#define DEBUG_H

#include <kdebug.h>

int kdeDebugArea();

#define mrimDebug() kDebug(kdeDebugArea())

#define mrimWarning() kWarning(kdeDebugArea())

#endif // DEBUG_H
