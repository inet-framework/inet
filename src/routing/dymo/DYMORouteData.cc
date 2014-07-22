//
// This program is property of its copyright holder. All rights reserved.
//

#include "DYMORouteData.h"

namespace inet {

namespace dymo {

DYMORouteData::DYMORouteData()
{
    isBroken = false;
    sequenceNumber = 0;
    lastUsed = 0;
    expirationTime = 0;
    metricType = HOP_COUNT;
}

} // namespace dymo

} // namespace inet

