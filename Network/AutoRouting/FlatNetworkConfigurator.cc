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

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include "RoutingTable.h"
#include "StringTokenizer.h"
#include "IPAddressResolver.h"
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

    // we'll store node addresses here
    std::vector<uint32> nodeAddresses;
    nodeAddresses.resize(topo.nodes());

    int i;
    int numIPNodes = 0;
    for (i=0; i<topo.nodes(); i++)
    {
        // skip bus types
        if (std::find(nonIPTypes.begin(), nonIPTypes.end(), topo.node(i)->module()->className())!=nonIPTypes.end())
            continue;

        uint32 addr = networkAddress | uint32(++numIPNodes);
        nodeAddresses[i] = addr;

        // find interface table and assign address to all (non-loopback) interfaces
        cModule *mod = topo.node(i)->module();
        RoutingTable *rt = IPAddressResolver().routingTableOf(mod);

        for (int k=0; k<rt->numInterfaces(); k++)
        {
            InterfaceEntry *e = rt->interfaceById(k);
            if (!e->loopback)
            {
                e->inetAddr = IPAddress(addr);
                e->mask = IPAddress("255.255.255.255"); // full address must match for local delivery
            }
        }
    }

    // add default route to nodes with exactly one (non-loopback) interface
    std::vector<bool> usesDefaultRoute;
    usesDefaultRoute.resize(topo.nodes());
    for (i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *node = topo.node(i);

        // skip bus types
        if (std::find(nonIPTypes.begin(), nonIPTypes.end(), topo.node(i)->module()->className())!=nonIPTypes.end())
            continue;

        RoutingTable *rt = IPAddressResolver().routingTableOf(node->module());

        // count non-loopback interfaces
        int numIntf = 0;
        InterfaceEntry *interf = NULL;
        for (int k=0; k<rt->numInterfaces(); k++)
            if (!rt->interfaceById(k)->loopback)
                {interf = rt->interfaceById(k); numIntf++;}

        usesDefaultRoute[i] = (numIntf==1);
        if (numIntf!=1)
            continue; // only deal with nodes with one interface plus loopback

        ev << "  " << node->module()->fullName() << "=" << IPAddress(nodeAddresses[i])
           << " has only one (non-loopback) interface, adding default route\n";

        // add route
        RoutingEntry *e = new RoutingEntry();
        e->host = IPAddress();
        e->netmask = IPAddress();
        e->interfaceName = interf->name.c_str();
        e->interfacePtr = interf;
        e->type = RoutingEntry::REMOTE;
        e->source = RoutingEntry::MANUAL;
        //e->metric = 1;
        rt->addRoutingEntry(e);
    }

    // fill in routing tables
    for (i=0; i<topo.nodes(); i++)
    {
        cTopology::Node *destNode = topo.node(i);

        // skip bus types
        if (std::find(nonIPTypes.begin(), nonIPTypes.end(), destNode->module()->className())!=nonIPTypes.end())
            continue;

        uint32 destAddr = nodeAddresses[i];
        std::string destModName = destNode->module()->fullName();

        // calculate shortest paths from everywhere towards destNode
        topo.unweightedSingleShortestPathsTo(destNode);

        // add route (with host=destNode) to every routing table in the network
        // (excepting nodes with only one interface -- there we'll set up a default route)
        for (int j=0; j<topo.nodes(); j++)
        {
            if (i==j) continue;
            if (std::find(nonIPTypes.begin(), nonIPTypes.end(), topo.node(j)->module()->className())!=nonIPTypes.end())
                continue;

            cTopology::Node *atNode = topo.node(j);
            if (atNode->paths()==0)
                continue; // not connected
            if (usesDefaultRoute[j])
                continue; // already added default route here

            uint32 atAddr = nodeAddresses[j];

            int outputPort = atNode->path(0)->localGate()->index();
            ev << "  from " << atNode->module()->fullName() << "=" << IPAddress(atAddr);
            ev << " towards " << destModName << "=" << IPAddress(destAddr) << " outputPort=" << outputPort << endl;

            // add route
            RoutingTable *rt = IPAddressResolver().routingTableOf(atNode->module());
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

    // update display string
    char buf[80];
    sprintf(buf, "%d IP nodes\n%d non-IP nodes", numIPNodes, topo.nodes()-numIPNodes);
    displayString().setTagArg("t",0,buf);

}

void FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}


