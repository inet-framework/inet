//
// Copyright (C) 2004 Andras Varga
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

#include <algorithm>
#include "RoutingTable.h"
#include "StringTokenizer.h"
#include "FlatNetworkConfigurator.h"


Define_Module(FlatNetworkConfigurator);


void FlatNetworkConfigurator::initialize(int stage)
{
    if (stage!=2) return;

    cTopology topo("topo");

    std::vector<std::string> types = StringTokenizer(par("moduleTypes"), " ").asVector();
    topo.extractByModuleType(types);
    ev << "cTopology found " << topo.nodes() << " nodes\n";

    // Although bus types are not auto-configured, FNC must still know them
    // since topology may depend on them.
    std::vector<std::string> nonIPTypes = StringTokenizer(par("nonIPModuleTypes"), " ").asVector();

    // assign IP addresses
    uint32 networkAddress = IPAddress(par("networkAddress").stringValue()).getInt();
    uint32 netmask = IPAddress(par("netmask").stringValue()).getInt();
    int maxNodes = (~netmask)-1;  // 0 and ffff have special meaning and cannot be used
    if (topo.nodes()>maxNodes)
        error("netmask too large, not enough addresses for all %d nodes", topo.nodes());

    int i;
    int hostCtr = 0;
    for (i=0; i<topo.nodes(); i++)
    {
        // skip bus types
        if (std::find(nonIPTypes.begin(), nonIPTypes.end(), topo.node(i)->module()->className())!=nonIPTypes.end())
            continue;

        uint32 addr = networkAddress | uint32(++hostCtr);

        // find interface table and assign address to all interfaces with an output port
        cModule *mod = topo.node(i)->module();
        RoutingTable *rt = findRoutingTable(mod);

        for (int k=0; k<rt->numInterfaces(); k++)
        {
            InterfaceEntry *e = rt->interfaceById(k);
            if (e->outputPort!=-1)
            {
                e->inetAddr = IPAddress(addr);
                e->mask = IPAddress(netmask);
            }
        }
    }

    // find and store next hops
    hostCtr=0;
    for (i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *destNode = topo.node(i);
        if (std::find(nonIPTypes.begin(), nonIPTypes.end(), destNode->module()->className())!=nonIPTypes.end())
            continue;

        uint32 destAddr = networkAddress | uint32(++hostCtr);
        std::string destModName = destNode->module()->fullName();

        topo.unweightedSingleShortestPathsTo(destNode);

        int hostCtr2 = 0;
        for (int j=0; j<topo.nodes(); j++)
        {
            if (i==j) continue;
            if (std::find(nonIPTypes.begin(), nonIPTypes.end(), topo.node(j)->module()->className())!=nonIPTypes.end())
                continue;

            cTopology::Node *atNode = topo.node(j);
            if (atNode->paths()==0)
                continue; // not connected
            uint32 atAddr = networkAddress | uint32(++hostCtr2);

            int outputPort = atNode->path(0)->localGate()->index();
            ev << "  from " << atNode->module()->fullName() << "=" << IPAddress(atAddr);
            ev << " towards " << destModName << "=" << IPAddress(destAddr) << " outputPort=" << outputPort << endl;

            // add route
            RoutingTable *rt = findRoutingTable(atNode->module());
            InterfaceEntry *interf = rt->interfaceByPortNo(outputPort);

            RoutingEntry *e = new RoutingEntry();
            e->host = IPAddress(destAddr);
            e->netmask = IPAddress(255,255,255,255); // full match needed
            e->interfaceName = interf->name.c_str();
            e->interfacePtr = interf;
            e->type = RoutingEntry::DIRECT;
            e->source = RoutingEntry::MANUAL;
            //e->metric = 1;
            rt->addRoutingEntry(e);
        }
    }
}

RoutingTable *FlatNetworkConfigurator::findRoutingTable(cModule *ipnode)
{
    // TBD this could be made more flexible...
    cModule *networkLayer = ipnode->submodule("networkLayer");
    RoutingTable *rt = !networkLayer ? NULL : dynamic_cast<RoutingTable *>(networkLayer->submodule("routingTable"));
    if (!rt)
        error("cannot find module networklayer.routingTable in node '%s'", ipnode->fullPath());
    return rt;
}

void FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}


