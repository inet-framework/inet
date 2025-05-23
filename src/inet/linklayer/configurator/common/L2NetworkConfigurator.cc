//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/common/L2NetworkConfigurator.h"

#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(L2NetworkConfigurator);

void L2NetworkConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        configuration = par("config");
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION)
        ensureConfigurationComputed(topology);
}

L2NetworkConfigurator::InterfaceInfo::InterfaceInfo(Node *node, Node *childNode, NetworkInterface *networkInterface)
{
    this->node = node;
    this->networkInterface = networkInterface;
    this->childNode = childNode;
}

void L2NetworkConfigurator::extractTopology(L2Topology& topology)
{
    topology.extractFromNetwork(Topology::selectTopologyNode);
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    if (topology.getNumNodes() == 0)
        throw cRuntimeError("Empty network!");

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        node->module = node->getModule();
        cModule *ifTable = node->module->getSubmodule("interfaceTable");

        // EthernetHost has no InterfaceTable
        if (ifTable) // todo:
            node->interfaceTable = dynamic_cast<IInterfaceTable *>(ifTable);
    }

    // extract links and interfaces
    std::set<NetworkInterface *> interfacesSeen;
    std::queue<Node *> Q; // unvisited nodes in the graph

    rootNode = (Node *)topology.getNode(0);
    Q.push(rootNode);

    while (!Q.empty()) {
        Node *node = Q.front();
        Q.pop();
        IInterfaceTable *interfaceTable = node->interfaceTable;

        if (interfaceTable) {
            // push neighbors to the queue
            for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
                NetworkInterface *networkInterface = interfaceTable->getInterface(i);
                if (interfacesSeen.count(networkInterface) == 0) {
                    // visiting this interface
                    interfacesSeen.insert(networkInterface);

                    Topology::Link *linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());

                    Node *childNode = nullptr;

                    if (linkOut) {
                        childNode = (Node *)linkOut->getLinkOutRemoteNode();
                        Q.push(childNode);
                    }

                    InterfaceInfo *info = new InterfaceInfo(node, childNode, networkInterface);
                    node->interfaceInfos.push_back(info);
                }
            }
        }
    }
}

void L2NetworkConfigurator::readInterfaceConfiguration(Node *rootNode)
{
    std::set<NetworkInterface *> matchedBefore;
    cXMLElementList interfaceElements = configuration->getChildrenByTagName("interface");

    for (auto& interfaceElements_i : interfaceElements) {
        std::set<NetworkInterface *> interfacesSeen;
        cXMLElement *interfaceElement = interfaceElements_i;

        const char *hostAttr = interfaceElement->getAttribute("hosts"); // "host* router[0..3]"
        const char *interfaceAttr = interfaceElement->getAttribute("names"); // i.e. interface names, like "eth* ppp0"
        const char *towardsAttr = interfaceElement->getAttribute("towards"); // neighbor host names, like "ap switch"
        const char *amongAttr = interfaceElement->getAttribute("among"); // neighbor host names, like "host[*] router1"
        const char *portsAttr = interfaceElement->getAttribute("ports"); // switch gate indices, like "0 1 2"

        // Begin RSTP properties, for more information see RSTP module
        const char *cost = interfaceElement->getAttribute("cost");
        const char *priority = interfaceElement->getAttribute("priority");
        const char *edge = interfaceElement->getAttribute("edge");
        // End RSTP properties

        if (amongAttr && *amongAttr) { // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s",
                        interfaceElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try {
            // parse host/interface/towards expressions
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);
            Matcher towardsMatcher(towardsAttr);
            Matcher portsMatcher(portsAttr);

            std::queue<Node *> Q;
            Q.push(rootNode);

            // configure port type/cost/priority constraints on matching interfaces
            while (!Q.empty()) {
                Node *currentNode = Q.front();
                Q.pop();

                for (unsigned int i = 0; i < currentNode->interfaceInfos.size(); i++) {
                    NetworkInterface *ifEntry = currentNode->interfaceInfos[i]->networkInterface;
                    if (interfacesSeen.count(ifEntry) == 0 && matchedBefore.count(ifEntry) == 0) {
                        cModule *hostModule = currentNode->module;
                        std::string hostFullPath = hostModule->getFullPath();
                        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                        // loopback interfaces
                        if (ifEntry->getNodeInputGateId() == -1) {
                            interfacesSeen.insert(ifEntry);
                            continue;
                        }

                        cGate *gate = hostModule->gate(ifEntry->getNodeInputGateId());
                        std::stringstream ss;
                        ss << gate->getIndex();
                        std::string port = ss.str();

                        // Note: "hosts", "interfaces" and "towards" must ALL match on the interface for the rule to apply
                        if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str()))
                            && (interfaceMatcher.matchesAny() || interfaceMatcher.matches(ifEntry->getInterfaceName()))
                            && (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(currentNode->interfaceInfos[i], towardsMatcher, hostModule))
                            && (portsMatcher.matchesAny() || portsMatcher.matches(port.c_str())))
                        {
                            // cost
                            if (!opp_isempty(cost))
                                currentNode->interfaceInfos[i]->portData.linkCost = atoi(cost);

                            // priority
                            if (!opp_isempty(priority))
                                currentNode->interfaceInfos[i]->portData.priority = atoi(priority);

                            // edge
                            if (!opp_isempty(edge))
                                currentNode->interfaceInfos[i]->portData.edge = strcmp(edge, "true") ? false : true;
                            EV_DEBUG << hostModule->getFullPath() << ":" << ifEntry->getInterfaceName() << endl;

                            matchedBefore.insert(ifEntry);
                        }

                        interfacesSeen.insert(ifEntry);
                        if (currentNode->interfaceInfos[i]->childNode)
                            Q.push(currentNode->interfaceInfos[i]->childNode);
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <interface> element at %s: %s", interfaceElement->getSourceLocation(),
                    e.what());
        }
    }
}

void L2NetworkConfigurator::computeConfiguration()
{
    long initializeStartTime = clock();
    // extract topology into the L2Topology object
    TIME(extractTopology(topology));
    // read the configuration from XML; it will serve as input for port assignment
    TIME(readInterfaceConfiguration(rootNode));
    printElapsedTime("initialize", initializeStartTime);
}

void L2NetworkConfigurator::ensureConfigurationComputed(L2Topology& topology)
{
    if (topology.getNumNodes() == 0)
        computeConfiguration();
}

Topology::Link *L2NetworkConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLinkOutLocalGateId() == gateId)
            return node->getLinkOut(i);

    return nullptr;
}

bool L2NetworkConfigurator::linkContainsMatchingHostExcept(InterfaceInfo *currentInfo, Matcher& hostMatcher,
        cModule *exceptModule)
{
    Node *childNode = currentInfo->childNode;

    if (childNode == nullptr)
        return false;

    cModule *hostModule = childNode->module;

    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

    if (hostModule == exceptModule)
        return false;

    if (hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str()))
        return true;

    return false;
}

void L2NetworkConfigurator::configureInterface(NetworkInterface *networkInterface)
{
    ensureConfigurationComputed(topology);
    cModule *networkNodeModule = findContainingNode(networkInterface);
    // TODO avoid linear search
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->module == networkNodeModule) {
            for (auto& elem : node->interfaceInfos) {
                InterfaceInfo *interfaceInfo = elem;
                if (interfaceInfo->networkInterface == networkInterface)
                    return configureInterface(interfaceInfo);
            }
        }
    }
}

void L2NetworkConfigurator::configureInterface(InterfaceInfo *interfaceInfo)
{
    NetworkInterface *networkInterface = interfaceInfo->networkInterface;
    auto interfaceData = networkInterface->getProtocolDataForUpdate<Ieee8021dInterfaceData>();

    interfaceData->setLinkCost(interfaceInfo->portData.linkCost);
    interfaceData->setPriority(interfaceInfo->portData.priority);
    interfaceData->setEdge(interfaceInfo->portData.edge);
}

L2NetworkConfigurator::Matcher::~Matcher()
{
    for (auto& elem : matchers)
        delete elem;
}

L2NetworkConfigurator::Matcher::Matcher(const char *pattern)
{
    matchesany = opp_isempty(pattern);

    if (matchesany)
        return;

    cStringTokenizer tokenizer(pattern);

    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

bool L2NetworkConfigurator::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;

    for (auto& elem : matchers)
        if (elem->matches(s))
            return true;

    return false;
}

} // namespace inet

