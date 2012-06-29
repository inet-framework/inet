//
// Copyright (C) 2010 Philipp Berndt
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

#ifndef __INET_GNPNETWORKCONFIGURATOR_H
#define __INET_GNPNETWORKCONFIGURATOR_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"
#include <boost/scoped_ptr.hpp>

namespace gnplib { namespace impl { namespace network { namespace gnp { class GnpNetLayerFactory; } } } }
class IInterfaceTable;
class IRoutingTable;


/**
 * Configures IP addresses and routing tables for a network.
 * IP-Adresses are chosen from a GNP hosts file.
 *
 * For more info please see the NED file.
 */
class INET_API GnpNetworkConfigurator : public cSimpleModule
{
  protected:
    struct NodeInfo {
        NodeInfo() {isIPNode=false;ift=NULL;rt=NULL;usesDefaultRoute=false;}
        bool isIPNode;
        IInterfaceTable *ift;
        IRoutingTable *rt;
        IPAddress address;
        bool usesDefaultRoute;
        std::string group;
    };
    typedef std::vector<NodeInfo> NodeInfoVector;

public:
    GnpNetworkConfigurator();
    virtual ~GnpNetworkConfigurator();
  protected:
    virtual int numInitStages() const  {return 3;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void extractTopology(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo);
    virtual void fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo);

    virtual void setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo);

private:
    GnpNetworkConfigurator(const GnpNetworkConfigurator& orig);
    boost::scoped_ptr<gnplib::impl::network::gnp::GnpNetLayerFactory> netLayerFactoryGnp;
};

#endif

