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

#include "RoutingTable.h"
#include "FlatNetworkConfigurator.h"


Define_Module(FlatNetworkConfigurator);


void FlatNetworkConfigurator::initialize(int stage)
{
    if (stage!=2) return;

    cTopology topo("topo");

    // FIXME use par("moduleTypesToProcess")
    topo.extractByModuleType("StandardHost2", "Router", NULL);
    ev << "cTopology found " << topo.nodes() << " nodes\n";

    // assign IP addresses
    uint32 networkAddress = IPAddress(par("networkAddress").stringValue()).getInt();
    uint32 netmask = IPAddress(par("netmask").stringValue()).getInt();
    int maxNodes = (~netmask)-1;  // 0 and ffff have special meaning and cannot be used
    if (topo.nodes()>maxNodes)
        error("netmask too large, not enough addresses for all %d nodes", topo.nodes());

    int i;
    for (i=0; i<topo.nodes(); i++)
    {
        // host part will be simply i+1.
        uint32 addr = networkAddress | uint32(i+1);

        // find interface table and assign address to all interfaces with an output port
        cModule *mod = topo.node(i)->module();
        RoutingTable *rt = findRoutingTable(mod);

        for (int k=0; k<rt->numInterfaces(); k++)
        {
            InterfaceEntry *e = rt->interfaceByIndex(k);
            if (e->outputPort!=-1)
            {
                e->inetAddr = IPAddress(addr);
                e->mask = IPAddress(255,255,255,255); // full match needed
            }
        }
    }

    // find and store next hops
    for (i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *destNode = topo.node(i);
        uint32 destAddr = networkAddress | uint32(i+1);
        std::string destModName = destNode->module()->fullName();

        topo.unweightedSingleShortestPathsTo(destNode);

        for (int j=0; j<topo.nodes(); j++)
        {
            if (i==j) continue;
            cTopology::Node *atNode = topo.node(j);
            if (atNode->paths()==0) continue; // not connected
            uint32 atAddr = networkAddress | uint32(j+1);

            int outputPort = atNode->path(0)->localGate()->index();
            ev << "  from " << atNode->module()->fullName() << "=" << IPAddress(atAddr);
            ev << " towards " << destModName << "=" << IPAddress(destAddr) << " outputPort=" << outputPort << endl;

            // add route
            RoutingTable *rt = findRoutingTable(atNode->module());
            InterfaceEntry *interf = rt->interfaceByPortNo(outputPort);

            RoutingEntry *e = new RoutingEntry();
            e->host = IPAddress(destAddr);
            e->netmask = IPAddress(netmask);
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
    // TBD could be made more flexible...
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


