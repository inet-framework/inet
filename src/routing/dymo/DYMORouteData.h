//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_DYMOROUTE_H_
#define __INET_DYMOROUTE_H_

#include <omnetpp.h>
#include "IRoute.h"
#include "DYMOdefs.h"

DYMO_NAMESPACE_BEGIN

/**
 * DYMO specific extra route data attached to routes in the routing table.
 */
class INET_API DYMORouteData : public cObject {
  private:
    bool isBroken;
    DYMOSequenceNumber sequenceNumber;
    simtime_t lastUsed;
    simtime_t expirationTime;
    DYMOMetricType metricType;

  public:
    DYMORouteData();
    virtual ~DYMORouteData() { }

    bool getBroken() const { return isBroken; }
    void setBroken(bool isBroken) { this->isBroken = isBroken; }

    DYMOSequenceNumber getSequenceNumber() const { return sequenceNumber; }
    void setSequenceNumber(DYMOSequenceNumber sequenceNumber) { this->sequenceNumber = sequenceNumber; }

    simtime_t getLastUsed() const { return lastUsed; }
    void setLastUsed(simtime_t lastUsed) { this->lastUsed = lastUsed; }

    simtime_t getExpirationTime() const { return expirationTime; }
    void setExpirationTime(simtime_t expirationTime) { this->expirationTime = expirationTime; }

    DYMOMetricType getMetricType() const { return metricType; }
    void setMetricType(DYMOMetricType metricType) { this->metricType = metricType; }
};

DYMO_NAMESPACE_END

#endif
