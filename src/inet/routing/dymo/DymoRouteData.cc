//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/dymo/DymoRouteData.h"

namespace inet {

namespace dymo {

std::string DymoRouteData::str() const
{
    std::ostringstream out;
    out << "isBroken = " << getBroken()
        << ", sequenceNumber = " << getSequenceNumber()
        << ", lastUsed = " << getLastUsed()
        << ", expirationTime = " << getExpirationTime()
        << ", metricType = " << getMetricType();
    return out.str();
};

} // namespace dymo
} // namespace inet

