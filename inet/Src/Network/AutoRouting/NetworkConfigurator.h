//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __NETWORKCONFIGURATOR_H__
#define __NETWORKCONFIGURATOR_H__

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"

class InterfaceTable;
class RoutingTable;


/**
 * Configures IP addresses and routing tables for a network.
 * For more info please see the NED file.
 */
class INET_API NetworkConfigurator : public cSimpleModule
{
  protected:
    struct NodeInfo {
        NodeInfo() {isIPNode=false;ift=NULL;rt=NULL;}
        bool isIPNode;
        InterfaceTable *ift;
        RoutingTable *rt;
        bool usesDefaultRoute;
    };
    typedef std::vector<NodeInfo> NodeInfoVector;
    typedef std::vector<std::string> StringVector;

  protected:
    virtual int numInitStages() const  {return 3;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    void extractTopology(cTopology& topo, NodeInfoVector& nodeInfo);
    void assignAddresses(cTopology& topo, NodeInfoVector& nodeInfo);
    void addPointToPointPeerRoutes(cTopology& topo, NodeInfoVector& nodeInfo);
    void addDefaultRoutes(cTopology& topo, NodeInfoVector& nodeInfo);
    void setPeersParameter(const char *submodName, cTopology& topo, NodeInfoVector& nodeInfo);
    void fillRoutingTables(cTopology& topo, NodeInfoVector& nodeInfo);

    void setDisplayString(cTopology& topo, NodeInfoVector& nodeInfo);
};

#endif

