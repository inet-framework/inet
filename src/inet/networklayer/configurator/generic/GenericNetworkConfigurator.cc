//
// Copyright (C) 2012 Opensim Ltd
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
//
// Authors: Levente Meszaros (primary author), Andras Varga, Tamas Borbely
//

#include "inet/networklayer/configurator/generic/GenericNetworkConfigurator.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"

namespace inet {

Define_Module(GenericNetworkConfigurator);

void GenericNetworkConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER_3) {
        long initializeStartTime = clock();
        Topology topology;
        // extract topology into the Topology object, then fill in a LinkInfo[] vector
        TIME(extractTopology(topology));
        // dump the result if requested
        if (par("dumpTopology").boolValue())
            TIME(dumpTopology(topology));
        // calculate shortest paths, and add corresponding static routes
        if (par("addStaticRoutes").boolValue())
            TIME(addStaticRoutes(topology));
        // dump routes to module output
        if (par("dumpRoutes").boolValue())
            TIME(dumpRoutes(topology));
        printElapsedTime("initialize", initializeStartTime);
    }
}

#undef T

IRoutingTable *GenericNetworkConfigurator::findRoutingTable(Node *node)
{
    return L3AddressResolver().findGenericRoutingTableOf(node->module);
}

void GenericNetworkConfigurator::addStaticRoutes(Topology& topology)
{
    // TODO: it should be configurable (via xml?) which nodes need static routes filled in automatically
    // add static routes for all routing tables
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *sourceNode = (Node *)topology.getNode(i);
        if (isBridgeNode(sourceNode))
            continue;
        GenericRoutingTable *sourceRoutingTable = dynamic_cast<GenericRoutingTable *>(sourceNode->routingTable);

        // calculate shortest paths from everywhere to sourceNode
        // we are going to use the paths in reverse direction (assuming all links are bidirectional)
        topology.calculateWeightedSingleShortestPathsTo(sourceNode);

        // add a route to all destinations in the network
        for (int j = 0; j < topology.getNumNodes(); j++) {
            // extract destination
            Node *destinationNode = (Node *)topology.getNode(j);
            if (sourceNode == destinationNode)
                continue;
            if (destinationNode->getNumPaths() == 0)
                continue;
            if (isBridgeNode(destinationNode))
                continue;

            //int destinationGateId = destinationNode->getPath(0)->getLocalGateId();
            IInterfaceTable *destinationInterfaceTable = destinationNode->interfaceTable;

            // determine next hop interface
            // find next hop interface (the last IP interface on the path that is not in the source node)
            Node *node = destinationNode;
            Link *link = nullptr;
            InterfaceInfo *nextHopInterfaceInfo = nullptr;
            while (node != sourceNode) {
                link = (Link *)node->getPath(0);
                if (node != sourceNode && !isBridgeNode(node) && link->sourceInterfaceInfo && link->sourceInterfaceInfo->interfaceEntry->getGenericNetworkProtocolData())
                    nextHopInterfaceInfo = static_cast<InterfaceInfo *>(link->sourceInterfaceInfo);
                node = (Node *)node->getPath(0)->getRemoteNode();
            }

            // determine source interface
            if (nextHopInterfaceInfo && link->destinationInterfaceInfo && link->destinationInterfaceInfo->addStaticRoute) {
                InterfaceEntry *nextHopInterfaceEntry = nextHopInterfaceInfo->interfaceEntry;
                InterfaceEntry *sourceInterfaceEntry = link->destinationInterfaceInfo->interfaceEntry;
                // add the same routes for all destination interfaces (IP packets are accepted from any interface at the destination)
                for (int j = 0; j < destinationInterfaceTable->getNumInterfaces(); j++) {
                    InterfaceEntry *destinationInterfaceEntry = destinationInterfaceTable->getInterface(j);
                    if (!destinationInterfaceEntry->getGenericNetworkProtocolData())
                        continue;
                    L3Address destinationAddress = destinationInterfaceEntry->getGenericNetworkProtocolData()->getAddress();
                    if (!destinationInterfaceEntry->isLoopback() && !destinationAddress.isUnspecified() && nextHopInterfaceEntry->getGenericNetworkProtocolData()) {
                        GenericRoute *route = new GenericRoute();
                        route->setSourceType(IRoute::MANUAL);
                        route->setDestination(destinationAddress);
                        route->setInterface(sourceInterfaceEntry);
                        L3Address nextHopAddress = nextHopInterfaceEntry->getGenericNetworkProtocolData()->getAddress();
                        if (nextHopAddress != destinationAddress)
                            route->setNextHop(nextHopAddress);
                        EV_DEBUG << "Adding route " << sourceInterfaceEntry->getFullPath() << " -> " << destinationInterfaceEntry->getFullPath() << " as " << route->info() << endl;
                        sourceRoutingTable->addRoute(route);
                    }
                }
            }
        }
    }
}

void GenericNetworkConfigurator::dumpRoutes(Topology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable) {
            EV_INFO << "Node " << node->module->getFullPath() << endl;
            node->routingTable->printRoutingTable();
            if (node->routingTable->getNumMulticastRoutes() > 0)
                ; // TODO: node->routingTable->printMulticastRoutingTable();
        }
    }
}

} // namespace inet

