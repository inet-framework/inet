//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"

#include <queue>
#include <set>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

void NetworkConfiguratorBase::extractTopology(Topology& topology)
{
    topology.extractByProperty("networkNode");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";
    if (topology.getNumNodes() == 0)
        throw cRuntimeError("Empty network!");
    // extract nodes, fill in interfaceTable and routingTable members in node
    L3AddressResolver addressResolver;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        node->module = node->getModule();
        node->interfaceTable = addressResolver.findInterfaceTableOf(node->module);
        node->routingTable = addressResolver.findIpv4RoutingTableOf(node->module);
    }
    // extract links and interfaces
    std::set<NetworkInterface *> interfacesSeen;
    std::queue<Node *> unvisited; // unvisited nodes in the graph
    auto rootNode = (Node *)topology.getNode(0);
    unvisited.push(rootNode);
    while (!unvisited.empty()) {
        Node *node = unvisited.front();
        unvisited.pop();
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable) {
            // push neighbors to the queue
            for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                NetworkInterface *networkInterface = interfaceTable->getInterface(i);
                if (interfacesSeen.count(networkInterface) == 0) {
                    // visiting this interface
                    interfacesSeen.insert(networkInterface);
                    Topology::LinkOut *linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
                    Node *childNode = nullptr;
                    if (linkOut) {
                        childNode = (Node *)linkOut->getRemoteNode();
                        unvisited.push(childNode);
                    }
                    Interface *interface = new Interface(node, networkInterface);
                    node->interfaces.push_back(interface);
                }
            }
        }
    }
    // annotate links with interfaces
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::LinkOut *linkOut = node->getLinkOut(j);
            Link *link = (Link *)linkOut;
            Node *localNode = (Node *)linkOut->getLocalNode();
            if (localNode->interfaceTable)
                link->sourceInterface = findInterface(localNode, localNode->interfaceTable->findInterfaceByNodeOutputGateId(linkOut->getLocalGateId()));
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            if (remoteNode->interfaceTable)
                link->destinationInterface = findInterface(remoteNode, remoteNode->interfaceTable->findInterfaceByNodeInputGateId(linkOut->getRemoteGateId()));
        }
    }
}

std::vector<NetworkConfiguratorBase::Node *> NetworkConfiguratorBase::computeShortestNodePath(Node *source, Node *destination) const
{
    std::vector<Node *> path;
    topology->calculateUnweightedSingleShortestPathsTo(destination);
    auto node = source;
    while (node != destination) {
        path.push_back(node);
        node = (Node *)node->getPath(0)->getRemoteNode();
    }
    path.push_back(destination);
    return path;
}

std::vector<NetworkConfiguratorBase::Link *> NetworkConfiguratorBase::computeShortestLinkPath(Node *source, Node *destination) const
{
    std::vector<Link *> path;
    topology->calculateUnweightedSingleShortestPathsTo(destination);
    auto node = source;
    while (node != destination) {
        auto link = (Link *)node->getPath(0);
        path.push_back(link);
        node = (Node *)node->getPath(0)->getRemoteNode();
    }
    return path;
}

bool NetworkConfiguratorBase::isBridgeNode(Node *node) const
{
    return !node->routingTable || !node->interfaceTable;
}

NetworkConfiguratorBase::Link *NetworkConfiguratorBase::findLinkIn(const Node *node, const char *neighbor) const
{
    for (int i = 0; i < node->getNumInLinks(); i++)
        if (!strcmp(node->getLinkIn(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkIn(i)));
    return nullptr;
}

NetworkConfiguratorBase::Link *NetworkConfiguratorBase::findLinkOut(const Node *node, const char *neighbor) const
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (!strcmp(node->getLinkOut(i)->getRemoteNode()->getModule()->getFullName(), neighbor))
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkOut(i)));
    return nullptr;
}

NetworkConfiguratorBase::Link *NetworkConfiguratorBase::findLinkOut(const Node *node, const Node *neighbor) const
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getRemoteNode() == neighbor)
            return check_and_cast<Link *>(static_cast<Topology::Link *>(node->getLinkOut(i)));
    return nullptr;
}

NetworkConfiguratorBase::Link *NetworkConfiguratorBase::findLinkOut(const Interface *interface) const
{
    for (int i = 0; i < interface->node->getNumOutLinks(); i++) {
        auto link = check_and_cast<Link *>(static_cast<Topology::Link *>(interface->node->getLinkOut(i)));
        if (link->sourceInterface == interface)
            return link;
    }
    return nullptr;
}

Topology::LinkOut *NetworkConfiguratorBase::findLinkOut(const Node *node, int gateId) const
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return nullptr;
}

NetworkConfiguratorBase::Interface *NetworkConfiguratorBase::findInterface(const Node *node, NetworkInterface *networkInterface) const
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interface : node->interfaces)
        if (interface->networkInterface == networkInterface)
            return interface;
    return nullptr;
}

} // namespace inet

