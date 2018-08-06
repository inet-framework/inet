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

#ifndef __INET_OSPFROUTEDATA_H
#define __INET_OSPFROUTEDATA_H

#include <set>
#include "inet/networklayer/common/L3Address.h"
#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv2/router/OspfRoutingTableEntry.h"

namespace inet {
namespace ospf {

class INET_API OspfRouteData : public cObject
{
  protected:
    OspfRoutingTableEntry::RoutingDestinationType destType;
    OspfRoutingTableEntry::RoutingPathType pathType;
    Ipv4Address area;

  public:

    OspfRouteData()
    {
        destType = 0;
        pathType = OspfRoutingTableEntry::INTRAAREA;
        area = Ipv4Address::UNSPECIFIED_ADDRESS;
    }

    virtual ~OspfRouteData() {}

    OspfRoutingTableEntry::RoutingDestinationType getDestType() const { return destType; }
    void setDestType(OspfRoutingTableEntry::RoutingDestinationType destType) { this->destType = destType; }
    OspfRoutingTableEntry::RoutingPathType getPathType() const { return pathType; }
    void setPathType(OspfRoutingTableEntry::RoutingPathType pathType) { this->pathType = pathType; }
    Ipv4Address getArea() const { return area; }
    void setArea(Ipv4Address area) { this->area = area; }
    virtual std::string str() const;
};

} // namespace ospf
} // namespace inet

#endif    // ifndef OSPFROUTEDATA_H_

