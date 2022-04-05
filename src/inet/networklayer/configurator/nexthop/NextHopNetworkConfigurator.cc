//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/nexthop/NextHopNetworkConfigurator.h"

#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#include "inet/networklayer/nexthop/NextHopRoutingTable.h"

namespace inet {

Define_Module(NextHopNetworkConfigurator);

void NextHopNetworkConfigurator::initialize(int stage)
{
    L3NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        long initializeStartTime = clock();
        Topology topology;
        // extract topology into the Topology object, then fill in a LinkInfo[] vector
        TIME(extractTopology(topology));
        // dump the result if requested
        if (par("dumpTopology"))
            TIME(dumpTopology(topology));
        // calculate shortest paths, and add corresponding static routes
        if (par("addStaticRoutes"))
            TIME(addStaticRoutes(topology));
        // dump routes to module output
        if (par("dumpRoutes"))
            TIME(dumpRoutes(topology));
        printElapsedTime("initialize", initializeStartTime);
    }
}

#undef T

IRoutingTable *NextHopNetworkConfigurator::findRoutingTable(Node *node)
{
    return L3AddressResolver().findNextHopRoutingTableOf(node->module);
}

void NextHopNetworkConfigurator::addStaticRoutes(Topology& topology)
{
    // TODO it should be configurable (via xml?) which nodes need static routes filled in automatically
    // add static routes for all routing tables
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *sourceNode = (Node *)topology.getNode(i);
        if (isBridgeNode(sourceNode))
            continue;
        NextHopRoutingTable *sourceRoutingTable = dynamic_cast<NextHopRoutingTable *>(sourceNode->routingTable);

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
            if (std::isinf(destinationNode->getDistanceToTarget()))
                continue;

//            int destinationGateId = destinationNode->getPath(0)->getLocalGateId();
            IInterfaceTable *destinationInterfaceTable = destinationNode->interfaceTable;

            // determine next hop interface
            // find next hop interface (the last IP interface on the path that is not in the source node)
            Node *node = destinationNode;
            Link *link = nullptr;
            InterfaceInfo *nextHopInterfaceInfo = nullptr;
            while (node != sourceNode) {
                link = (Link *)node->getPath(0);
                if (node != sourceNode && !isBridgeNode(node) && link->sourceInterfaceInfo && link->sourceInterfaceInfo->networkInterface->findProtocolData<NextHopInterfaceData>())
                    nextHopInterfaceInfo = static_cast<InterfaceInfo *>(link->sourceInterfaceInfo);
                node = (Node *)node->getPath(0)->getLinkOutRemoteNode();
            }

            // determine source interface
            if (nextHopInterfaceInfo && link->destinationInterfaceInfo && link->destinationInterfaceInfo->addStaticRoute) {
                NetworkInterface *nextHopNetworkInterface = nextHopInterfaceInfo->networkInterface;
                NetworkInterface *sourceNetworkInterface = link->destinationInterfaceInfo->networkInterface;
                // add the same routes for all destination interfaces (IP packets are accepted from any interface at the destination)
                for (int j = 0; j < destinationInterfaceTable->getNumInterfaces(); j++) {
                    NetworkInterface *destinationNetworkInterface = destinationInterfaceTable->getInterface(j);
                    auto destIeNextHopInterfaceData = destinationNetworkInterface->findProtocolData<NextHopInterfaceData>();
                    if (destIeNextHopInterfaceData == nullptr)
                        continue;
                    L3Address destinationAddress = destIeNextHopInterfaceData->getAddress();
                    if (!destinationNetworkInterface->isLoopback() && !destinationAddress.isUnspecified() && nextHopNetworkInterface->findProtocolData<NextHopInterfaceData>()) {
                        NextHopRoute *route = new NextHopRoute();
                        route->setSourceType(IRoute::MANUAL);
                        route->setDestination(destinationAddress);
                        route->setInterface(sourceNetworkInterface);
                        L3Address nextHopAddress = nextHopNetworkInterface->getProtocolData<NextHopInterfaceData>()->getAddress();
                        if (nextHopAddress != destinationAddress)
                            route->setNextHop(nextHopAddress);
                        EV_DEBUG << "Adding route " << sourceNetworkInterface->getInterfaceFullPath() << " -> " << destinationNetworkInterface->getInterfaceFullPath() << " as " << route->str() << endl;
                        sourceRoutingTable->addRoute(route);
                    }
                }
            }
        }
    }
}

void NextHopNetworkConfigurator::dumpRoutes(Topology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable) {
            EV_INFO << "Node " << node->module->getFullPath() << endl;
            node->routingTable->printRoutingTable();
            if (node->routingTable->getNumMulticastRoutes() > 0)
                ; // TODO node->routingTable->printMulticastRoutingTable();
        }
    }
}

} // namespace inet

