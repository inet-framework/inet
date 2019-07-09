//
// Copyright (C) 2014 OpenSim Ltd.
// Author: Benjamin Seregi
// Copyright (C) 2019 Universidad de Malaga
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_AODVROUTEDATA_H
#define __INET_AODVROUTEDATA_H

#include <set>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {
namespace inetmanet {

class INET_API LoadNgRouteData : public cObject
{
  protected:
    bool active = true;
    bool repariable = false;
    bool beingRepaired = false;
    bool bidirectional = false;

    int64_t destSeqNum = -1;
    int metricType = -1;
    unsigned int metric = 255;
    unsigned int hopCount = 0;

    simtime_t lifeTime = SIMTIME_ZERO;    // expiration or deletion time of the route

  public:

    LoadNgRouteData()
    {
    }

    virtual ~LoadNgRouteData() {}

    int64_t getDestSeqNum() const { return destSeqNum; }
    void setDestSeqNum(int64_t destSeqNum) { this->destSeqNum = destSeqNum; }

    bool hasValidDestNum() const { return destSeqNum != -1; }
    bool isBeingRepaired() const { return beingRepaired;}
    void setIsBeingRepaired(bool isBeingRepaired) { this->beingRepaired = isBeingRepaired; }
    bool isRepariable() const { return repariable; }
    void setIsRepariable(bool isRepariable) { this->repariable = isRepariable; }
    const simtime_t& getLifeTime() const { return lifeTime; }
    void setLifeTime(const simtime_t& lifeTime) { this->lifeTime = lifeTime; }
    bool isActive() const { return active; }
    void setIsActive(bool active) { this->active = active; }
    bool getIsBidirectiona() {return this->bidirectional;}
    void setIsBidirectiona(bool bidir) {this->bidirectional = bidir;}

    int getMetricType() {return this->metricType;}
    void setMetricType(int type) {this->metricType = type;}

    unsigned int getMetric() {return this->metric;}
    void setMetric(unsigned int val) {this->metric = val;}

    unsigned int getHopCount() {return this->hopCount;}
    void setHopCount(unsigned int val) {this->hopCount = val;}

    virtual std::string str() const;
};

} // namespace aodv
} // namespace inet

#endif    // ifndef AODVROUTEDATA_H_

