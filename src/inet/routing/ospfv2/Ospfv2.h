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

#ifndef __INET_OSPFV2_H
#define __INET_OSPFV2_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

/**
 * Implements the OSPFv2 routing protocol. See the NED file for more information.
 */
class Ospfv2 : public RoutingProtocolBase, protected cListener
{
  private:
    cModule *host = nullptr;    // the host module that owns this module
    IIpv4RoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;
    Router *ospfRouter = nullptr;    // root object of the OSPF data structure
    cMessage *startupTimer = nullptr;    // timer for delayed startup

  public:
    Ospfv2();
    virtual ~Ospfv2();

    /**
     * Insert a route learn by BGP in OSPF routingTable as an external route.
     * Used by the Bgp module.
     * @ifIndex: interface ID
     */
    void insertExternalRoute(int ifIndex, const Ipv4AddressRange& netAddr);

    /**
     * Checks if the route is in OSPF external LSA Table.
     * 0: not external, 1: type 1 external, 2: type 2 external
     * Used by the Bgp module.
     */
    int checkExternalRoute(const Ipv4Address& route);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void subscribe();
    virtual void unsubscribe();
    virtual void createOspfRouter();

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    void handleInterfaceDown(const InterfaceEntry *ie);
};

} // namespace ospfv2

} // namespace inet

#endif // ifndef __INET_OSPFV2_H

