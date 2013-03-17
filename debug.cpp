#include <kdebug.h>

#include "debug.h"

int kdeDebugArea() {
    static int area = KDebug::registerArea("kopete (kopete_mrim)");
    return area;
}
