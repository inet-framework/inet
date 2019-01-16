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

#include "inet/common/INETDefs.h"
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

#endif // ifndef __INET_DYMOROUTEDATA_H

