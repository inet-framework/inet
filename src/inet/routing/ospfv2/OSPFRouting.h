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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/routing/ospfv2/OSPFPacket_m.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

namespace ospf {

/**
 * Implements the OSPFv2 routing protocol. See the NED file for more information.
 */
class OSPFRouting : public cSimpleModule, public ILifecycle
{
  private:
    IIPv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    bool isUp = false;
    Router *ospfRouter = nullptr;    // root object of the OSPF data structure

  public:
    OSPFRouting();
    virtual ~OSPFRouting();

    /**
     * Insert a route learn by BGP in OSPF routingTable as an external route.
     * Used by the BGPRouting module.
     * @ifIndex: interface ID
     */
    void insertExternalRoute(int ifIndex, const IPv4AddressRange& netAddr);

    /**
     * Return true if the route is in OSPF external LSA Table, false else.
     * Used by the BGPRouting module.
     */
    bool checkExternalRoute(const IPv4Address& route);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenDown(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void createOspfRouter();
    virtual bool isNodeUp();
};

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFROUTING_H

