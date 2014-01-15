//
// This program is property of its copyright holder. All rights reserved.
//

#include "DYMORouteData.h"

DYMO_NAMESPACE_BEGIN

DYMORouteData::DYMORouteData() {
    isBroken = false;
    sequenceNumber = 0;
    lastUsed = 0;
    expirationTime = 0;
    metricType = HOP_COUNT;
}

DYMO_NAMESPACE_END
