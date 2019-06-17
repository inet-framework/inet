//
// Copyright (C) 2012 Univerdidad de Malaga.
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

#ifndef WIRELESSGETNEIG_H_
#define WIRELESSGETNEIG_H_

#include <vector>
#include <map>
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/geometry/common/EulerAngles.h"

namespace inet{

class IInterfaceTable;
class IMobility;

class WirelessGetNeig : public cOwnedObject
{
        struct nodeInfo
        {
            IMobility* mob;
            IInterfaceTable* itable;
        };
        typedef std::map<uint32_t,nodeInfo> ListNodes;
        typedef std::map<MacAddress,nodeInfo> ListNodesMac;

        ListNodes listNodes;
        ListNodesMac listNodesMac;
    public:
        WirelessGetNeig();
        virtual ~WirelessGetNeig();
        virtual void getNeighbours(const Ipv4Address &node, std::vector<Ipv4Address>&, const double &distance);
        virtual void getNeighbours(const MacAddress &node, std::vector<MacAddress>&, const double &distance);
        virtual void getNeighbours(const Ipv4Address &node, std::vector<Ipv4Address>&, const double &distance, std::vector<Coord> &);
        virtual void getNeighbours(const MacAddress &node, std::vector<MacAddress>&, const double &distance, std::vector<Coord> &);
        virtual EulerAngles getDirection(const Ipv4Address &, const Ipv4Address&, double &distance);
        virtual EulerAngles getDirection(const MacAddress &, const MacAddress&, double &distance);
};


}

#endif /* WIRELESSNUMHOPS_H_ */
