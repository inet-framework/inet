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

#pragma warning(disable:4786)



Define_Module(FlatNetworkConfigurator);


void FlatNetworkConfigurator::initialize(int stage)
{
    if (stage!=2) return;

    cTopology *topo = new cTopology("topo");

    // this can probably be made more flexible
    topo->extractByModuleType(par("nodeType"), NULL);
    ev << "cTopology found " << topo->nodes() << " nodes\n";

    // find and store next hops
    //
    // note that we do route calculation (call unweightedSingleShortestPathsTo())
    // only n times, while per-node calculation would require n^2 invocations.
    //
    for (int i=0; i<topo->nodes(); i++)
    {
        RTEntry key;
        cTopology::Node *destNode = topo->node(i);
        key.destAddress = destNode->module()->par("address");

        topo->unweightedSingleShortestPathsTo(destNode);

        for (int j=0; j<topo->nodes(); j++)
        {
            if (i==j) continue;
            cTopology::Node *atNode = topo->node(j);
            if (atNode->paths()==0) continue; // not connected

            key.atAddress = atNode->module()->par("address");
            int gateId = atNode->path(0)->localGate()->id();
            rtable[key] = gateId;
            ev << "  from " << key.atAddress << " towards " << key.destAddress << " gateId is " << gateId << endl;
        }
    }
    delete topo;
}

int FlatNetworkConfigurator::getNextHop(int atAddress, int destAddress)
{
    RTEntry key;
    key.atAddress = atAddress;
    key.destAddress = destAddress;

    RoutingTable::iterator it = rtable.find(key);
    if (it==rtable.end())
        return -1;

    int outGate = (*it).second;
    return outGate;
}

void FlatNetworkConfigurator::handleMessage(cMessage *msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}


