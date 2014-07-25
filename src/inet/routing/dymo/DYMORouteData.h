//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_DYMOROUTEDATA_H
#define __INET_DYMOROUTEDATA_H

#include <omnetpp.h>
#include "inet/networklayer/common/IRoute.h"
#include "inet/routing/dymo/DYMOdefs.h"

namespace inet {

namespace dymo {

/**
 * DYMO-specific extra route data attached to routes in the routing table.
 */
class INET_API DYMORouteData : public cObject
{
  private:
    bool isBroken;
    DYMOSequenceNumber sequenceNumber;
    simtime_t lastUsed;
    simtime_t expirationTime;
    DYMOMetricType metricType;

  public:
    DYMORouteData();
    virtual ~DYMORouteData() {}

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

} // namespace dymo

} // namespace inet

#endif // ifndef __INET_DYMOROUTEDATA_H

