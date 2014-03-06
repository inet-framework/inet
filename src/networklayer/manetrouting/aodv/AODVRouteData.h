//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef AODVROUTEDATA_H_
#define AODVROUTEDATA_H_

#include <vector>
#include "Address.h"
#include "INETDefs.h"

class INET_API AODVRouteData : public cObject
{
    protected:
        std::vector<Address> precursorList;
        bool active;
        bool repariable;
        bool beingRepaired;
        bool validDestNum;
        unsigned int destSeqNum;
        simtime_t lifeTime; // expiration or deletion time of the route
        simtime_t lastUsed; // the last time when the route was accessed

    public:

        AODVRouteData() { destSeqNum = 0; }
        virtual ~AODVRouteData() {}

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
        void setLastUsed(const simtime_t& lastUsed) { this->lastUsed = lastUsed; }
        const simtime_t& getLastUsed() const { return lastUsed; }
        bool isActive() const { return active; }
        void setIsActive(bool active) { this->active = active; }
        void addPrecursor(const Address& precursorAddr) { precursorList.push_back(precursorAddr); }
        const std::vector<Address>& getPrecursorList() { return precursorList; }
};

#endif
