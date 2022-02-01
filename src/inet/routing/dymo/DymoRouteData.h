//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DYMOROUTEDATA_H
#define __INET_DYMOROUTEDATA_H

#include "inet/networklayer/contract/IRoute.h"
#include "inet/routing/dymo/Dymo_m.h"

namespace inet {
namespace dymo {

/**
 * Dymo-specific extra route data attached to routes in the routing table.
 */
class INET_API DymoRouteData : public cObject
{
  private:
    bool isBroken;
    DymoSequenceNumber sequenceNumber;
    simtime_t lastUsed;
    simtime_t expirationTime;
    DymoMetricType metricType;

  public:
    DymoRouteData();
    virtual ~DymoRouteData() {}

    bool getBroken() const { return isBroken; }
    void setBroken(bool isBroken) { this->isBroken = isBroken; }

    DymoSequenceNumber getSequenceNumber() const { return sequenceNumber; }
    void setSequenceNumber(DymoSequenceNumber sequenceNumber) { this->sequenceNumber = sequenceNumber; }

    simtime_t getLastUsed() const { return lastUsed; }
    void setLastUsed(simtime_t lastUsed) { this->lastUsed = lastUsed; }

    simtime_t getExpirationTime() const { return expirationTime; }
    void setExpirationTime(simtime_t expirationTime) { this->expirationTime = expirationTime; }

    DymoMetricType getMetricType() const { return metricType; }
    void setMetricType(DymoMetricType metricType) { this->metricType = metricType; }

    virtual std::string str() const;
};

} // namespace dymo
} // namespace inet

#endif

