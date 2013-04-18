//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSPFROUTING_H
#define __INET_OSPFROUTING_H


#include <vector>

#include "INETDefs.h"

#include "IInterfaceTable.h"
#include "IRoutingTable.h"
#include "OSPFPacket_m.h"
#include "OSPFRouter.h"


/**
 * Implements the OSPFv2 routing protocol. See the NED file for more information.
 */
class OSPFRouting :  public cSimpleModule
{
  private:
    OSPF::Router *ospfRouter; // root object of the OSPF data structure

  public:
    OSPFRouting();
    virtual ~OSPFRouting();

    /**
     * Insert a route learn by BGP in OSPF routingTable as an external route.
     * Used by the BGPRouting module.
     * @ifIndex: interface ID
     */
    void insertExternalRoute(int ifIndex, const OSPF::IPv4AddressRange& netAddr);

    /**
     * Return true if the route is in OSPF external LSA Table, false else.
     * Used by the BGPRouting module.
     */
    bool checkExternalRoute(const IPv4Address& route);

  protected:
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif  // __INET_OSPFROUTING_H


