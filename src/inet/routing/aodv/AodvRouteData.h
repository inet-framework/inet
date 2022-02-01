//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_AODVROUTEDATA_H
#define __INET_AODVROUTEDATA_H

#include <set>

#include "inet/networklayer/common/L3Address.h"

namespace inet {
namespace aodv {

class INET_API AodvRouteData : public cObject
{
  protected:
    std::set<L3Address> precursorList;
    bool active;
    bool repariable;
    bool beingRepaired;
    bool validDestNum;
    unsigned int destSeqNum;
    simtime_t lifeTime; // expiration or deletion time of the route

  public:

    AodvRouteData()
    {
        active = true;
        repariable = false;
        beingRepaired = false;
        validDestNum = true;
        lifeTime = SIMTIME_ZERO;
        destSeqNum = 0;
    }

    virtual ~AodvRouteData() {}

    unsigned int getDestSeqNum() const { return destSeqNum; }
    void setDestSeqNum(unsigned int destSeqNum) { this->destSeqNum = destSeqNum; }
    bool hasValidDestNum() const { return validDestNum; }
    void setHasValidDestNum(bool hasValidDestNum) { this->validDestNum = hasValidDestNum; }
    bool isBeingRepaired() const { return beingRepaired; }
    void setIsBeingRepaired(bool isBeingRepaired) { this->beingRepaired = isBeingRepaired; }
    bool isRepariable() const { return repariable; }
    void setIsRepariable(bool isRepariable) { this->repariable = isRepariable; }
    const simtime_t& getLifeTime() const { return lifeTime; }
    void setLifeTime(const simtime_t& lifeTime) { this->lifeTime = lifeTime; }
    bool isActive() const { return active; }
    void setIsActive(bool active) { this->active = active; }
    void addPrecursor(const L3Address& precursorAddr) { precursorList.insert(precursorAddr); }
    const std::set<L3Address>& getPrecursorList() const { return precursorList; }
    virtual std::string str() const;
};

} // namespace aodv
} // namespace inet

#endif

