//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_FLATNETWORKCONFIGURATOR_H
#define __INET_FLATNETWORKCONFIGURATOR_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {

class IInterfaceTable;
class IIPv4RoutingTable;

/**
 * Configures IPv4 addresses and routing tables for a "flat" network,
 * "flat" meaning that all hosts and routers will have the same
 * network address.
 *
 * For more info please see the NED file.
 */
class INET_API FlatNetworkConfigurator : public cSimpleModule
{
  protected:
    class NodeInfo
    {
      public:
        bool isIPNode = false;
        IInterfaceTable *ift = nullptr;
        IIPv4RoutingTable *rt = nullptr;
        IPv4Address address;
        bool usesDefaultRoute = false;
        bool ipForwardEnabled = false;
    };
    typedef std::vector<NodeInfo> NodeInfoVector;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void extractTopology(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo);

    virtual void setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo);
};

} // namespace inet

#endif // ifndef __INET_FLATNETWORKCONFIGURATOR_H

