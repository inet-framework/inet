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

#include <set>
#include "inet/common/stlutils.h"
#include "inet/networklayer/generic/GenericRoutingTable.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/configurator/generic/GenericNetworkConfigurator.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(GenericNetworkConfigurator);

inline bool isEmpty(const char *s) { return !s || !s[0]; }
inline bool isNotEmpty(const char *s) { return s && s[0]; }

static void printTimeSpentUsingDuration(const char *name, long duration)
{
    EV_INFO << "Time spent in GenericNetworkConfigurator::" << name << ": " << ((double)duration / CLOCKS_PER_SEC) << "s" << endl;
}

static void printElapsedTime(const char *name, long startTime)
{
    printTimeSpentUsingDuration(name, clock() - startTime);
}

#define T(CODE)    { long startTime = clock(); CODE; printElapsedTime( #CODE, startTime); }
void GenericNetworkConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER) {
        long initializeStartTime = clock();

        GenericTopology topology;

        // extract topology into the GenericTopology object, then fill in a LinkInfo[] vector
        T(extractTopology(topology));

        // dump the result if requested
        if (par("dumpTopology").boolValue())
            T(dumpTopology(topology));

        // calculate shortest paths, and add corresponding static routes
        if (par("addStaticRoutes").boolValue())
            T(addStaticRoutes(topology));

        // dump routes to module output
        if (par("dumpRoutes").boolValue())
            T(dumpRoutes(topology));

        printElapsedTime("initialize", initializeStartTime);
    }
}

#undef T

void GenericNetworkConfigurator::extractTopology(GenericTopology& topology)
{
    // TODO: the current topology discovery implementation doesn't support heterogeneous wired and wireless links
    extractWiredTopology(topology);
    extractWirelessTopology(topology);

    // determine gatewayInterfaceInfo for all linkInfos
    for (int linkIndex = 0; linkIndex < (int)topology.linkInfos.size(); linkIndex++) {
        LinkInfo *linkInfo = topology.linkInfos[linkIndex];
        linkInfo->gatewayInterfaceInfo = determineGatewayForLink(linkInfo);
    }
}

bool GenericNetworkConfigurator::isWirelessInterface(InterfaceEntry *interfaceEntry)
{
    return !strncmp(interfaceEntry->getName(), "wlan", 4);
}

Topology::LinkOut *GenericNetworkConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);

    return NULL;
}

GenericNetworkConfigurator::InterfaceInfo *GenericNetworkConfigurator::findInterfaceInfo(Node *node, InterfaceEntry *ie)
{
    for (int i = 0; i < (int)node->interfaceInfos.size(); i++)
        if (node->interfaceInfos.at(i)->interfaceEntry == ie)
            return node->interfaceInfos.at(i);

    return NULL;
}

void GenericNetworkConfigurator::extractWiredTopology(GenericTopology& topology)
{
    // extract topology
    topology.extractByProperty("node");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        node->module = module;
        node->interfaceTable = L3AddressResolver().findInterfaceTableOf(module);
        node->routingTable = L3AddressResolver().findGenericRoutingTableOf(module);
        if (node->routingTable && !node->routingTable->isForwardingEnabled())
            node->setWeight(DBL_MAX);
    }

    // extract links and interfaces
    std::set<InterfaceEntry *> interfacesSeen;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        IInterfaceTable *interfaceTable = L3AddressResolver().findInterfaceTableOf(module);
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *ie = interfaceTable->getInterface(j);
                if (!ie->isLoopback() && interfacesSeen.count(ie) == 0 && !isWirelessInterface(ie)) {
                    // create a new network link
                    LinkInfo *linkInfo = new LinkInfo(false);
                    topology.linkInfos.push_back(linkInfo);

                    // store interface as belonging to the new network link
                    InterfaceInfo *interfaceInfo = createInterfaceInfo(node, linkInfo, ie);
                    linkInfo->interfaceInfos.push_back(interfaceInfo);
                    interfacesSeen.insert(ie);

                    // visit neighbor (and potentially the whole LAN, recursively)
                    Topology::LinkOut *linkOut = findLinkOut((Node *)topology.getNode(i), ie->getNodeOutputGateId());
                    if (linkOut) {
                        std::vector<Node *> empty;
                        extractWiredNeighbors(linkOut, linkInfo, interfacesSeen, empty);
                    }
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
                link->sourceInterfaceInfo = findInterfaceInfo(localNode, localNode->interfaceTable->getInterfaceByNodeOutputGateId(linkOut->getLocalGateId()));
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            if (remoteNode->interfaceTable)
                link->destinationInterfaceInfo = findInterfaceInfo(remoteNode, remoteNode->interfaceTable->getInterfaceByNodeInputGateId(linkOut->getRemoteGateId()));
        }
    }
}

void GenericNetworkConfigurator::extractWiredNeighbors(Topology::LinkOut *linkOut, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    cChannel *transmissionChannel = linkOut->getLocalGate()->getTransmissionChannel();
    if (transmissionChannel)
        linkOut->setWeight(getChannelWeight(transmissionChannel));

    Node *neighborNode = (Node *)linkOut->getRemoteNode();
    cModule *neighborModule = neighborNode->getModule();
    int neighborInputGateId = linkOut->getRemoteGateId();
    IInterfaceTable *neighborInterfaceTable = L3AddressResolver().findInterfaceTableOf(neighborModule);
    if (neighborInterfaceTable) {
        // neighbor is a host or router, just add the interface
        InterfaceEntry *neighborInterfaceEntry = neighborInterfaceTable->getInterfaceByNodeInputGateId(neighborInputGateId);
        if (interfacesSeen.count(neighborInterfaceEntry) == 0) {
            InterfaceInfo *neighborInterfaceInfo = createInterfaceInfo(neighborNode, linkInfo, neighborInterfaceEntry);
            linkInfo->interfaceInfos.push_back(neighborInterfaceInfo);
            interfacesSeen.insert(neighborInterfaceEntry);
        }
    }
    else {
        // assume that neighbor is an L2 or L1 device (bus/hub/switch/bridge/access point/etc); visit all its output links
        Node *deviceNode = (Node *)linkOut->getRemoteNode();
        if (!contains(deviceNodesVisited, deviceNode)) {
            deviceNodesVisited.push_back(deviceNode);
            for (int i = 0; i < deviceNode->getNumOutLinks(); i++) {
                Topology::LinkOut *deviceLinkOut = deviceNode->getLinkOut(i);
                extractWiredNeighbors(deviceLinkOut, linkInfo, interfacesSeen, deviceNodesVisited);
            }
        }
    }
}

double GenericNetworkConfigurator::getChannelWeight(cChannel *transmissionChannel)
{
    //TODO shouldn't we use interface metric here?
    double datarate = transmissionChannel->getNominalDatarate();
    return datarate > 0 ? 1 / datarate : 1.0;    //TODO why 1.0 if there's no datarate?
}

/**
 * Discover wireless LANs, and create a corresponding LinkInfo for each.
 * We use getWirelessId() on all wireless interfaces to try and determine the
 * WLAN "id" (that's SSID for 802.11) they're on.
 */
void GenericNetworkConfigurator::extractWirelessTopology(GenericTopology& topology)
{
    std::map<std::string, LinkInfo *> wirelessIdToLinkInfoMap;    // wireless LANs by name

    // iterate through all wireless interfaces and determine the wireless id.
    for (int nodeIndex = 0; nodeIndex < topology.getNumNodes(); nodeIndex++) {
        Node *node = (Node *)topology.getNode(nodeIndex);
        cModule *module = node->getModule();
        IInterfaceTable *interfaceTable = L3AddressResolver().findInterfaceTableOf(module);
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *ie = interfaceTable->getInterface(j);
                if (!ie->isLoopback() && isWirelessInterface(ie)) {
                    const char *wirelessId = getWirelessId(ie);
                    EV_DEBUG << "Interface " << ie->getFullPath() << " has wireless id " << wirelessId << endl;
                    LinkInfo *linkInfo = wirelessIdToLinkInfoMap[wirelessId];
                    if (!linkInfo) {
                        linkInfo = new LinkInfo(true);
                        topology.linkInfos.push_back(linkInfo);
                        wirelessIdToLinkInfoMap[wirelessId] = linkInfo;
                    }
                    InterfaceInfo *interfaceInfo = createInterfaceInfo(node, linkInfo, ie);
                    linkInfo->interfaceInfos.push_back(interfaceInfo);
                }
            }
        }
    }

    // add links between all pairs of wireless interfaces (full graph)
    for (std::map<std::string, LinkInfo *>::iterator it = wirelessIdToLinkInfoMap.begin(); it != wirelessIdToLinkInfoMap.end(); it++) {
        LinkInfo *linkInfo = it->second;
        for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++) {
            InterfaceInfo *interfaceInfoI = linkInfo->interfaceInfos.at(i);
            for (int j = i + 1; j < (int)linkInfo->interfaceInfos.size(); j++) {
                InterfaceInfo *interfaceInfoJ = linkInfo->interfaceInfos.at(j);
                Link *link = new Link();
                link->sourceInterfaceInfo = interfaceInfoI;
                link->destinationInterfaceInfo = interfaceInfoJ;
                topology.addLink(link, interfaceInfoI->node, interfaceInfoJ->node);
                link = new Link();
                link->sourceInterfaceInfo = interfaceInfoJ;
                link->destinationInterfaceInfo = interfaceInfoI;
                topology.addLink(link, interfaceInfoJ->node, interfaceInfoI->node);
            }
        }
    }
}

/**
 * If this function returns the same string for two wireless interfaces, they
 * will be regarded as being in the same wireless network. (The actual value
 * of the string doesn't count.)
 */
const char *GenericNetworkConfigurator::getWirelessId(InterfaceEntry *interfaceEntry)
{
    // use the configuration
    cModule *hostModule = interfaceEntry->getInterfaceTable()->getHostModule();
    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
    cXMLElement *root = par("config").xmlValue();
    cXMLElementList wirelessElements = root->getChildrenByTagName("wireless");
    for (int i = 0; i < (int)wirelessElements.size(); i++) {
        cXMLElement *wirelessElement = wirelessElements[i];
        const char *hostAttr = wirelessElement->getAttribute("hosts");    // "host* router[0..3]"
        const char *interfaceAttr = wirelessElement->getAttribute("interfaces");    // i.e. interface names, like "eth* ppp0"
        try {
            // parse host/interface expressions
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);

            // Note: "hosts", "interfaces" must ALL match on the interface for the rule to apply
            if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceEntry->getFullName())))
            {
                const char *idAttr = wirelessElement->getAttribute("id");    // identifier of wireless connection
                return idAttr ? idAttr : wirelessElement->getSourceLocation();
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <wireless> element at %s: %s", wirelessElement->getSourceLocation(), e.what());
        }
    }

    // if the mgmt submodule within the wireless NIC has an "ssid" or "accessPointAddress" parameter, we can use that
    cModule *module = interfaceEntry->getInterfaceModule();
    if (!module)
        module = hostModule;
    cSimpleModule *mgmtModule = ModuleAccess<cSimpleModule>("mgmt").getIfExists(module);
    if (mgmtModule) {
        if (mgmtModule->hasPar("ssid"))
            return mgmtModule->par("ssid");
        else if (mgmtModule->hasPar("accessPointAddress"))
            return mgmtModule->par("accessPointAddress");
    }

    // default: put all such wireless interfaces on the same LAN
    return "SSID";
}

/**
 * If this link has exactly one node that connects to other links as well, we can assume
 * it is a "gateway" and return that (we'll use it in routing); otherwise return NULL.
 */
GenericNetworkConfigurator::InterfaceInfo *GenericNetworkConfigurator::determineGatewayForLink(LinkInfo *linkInfo)
{
    InterfaceInfo *gatewayInterfaceInfo = NULL;
    for (int interfaceIndex = 0; interfaceIndex < (int)linkInfo->interfaceInfos.size(); interfaceIndex++) {
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[interfaceIndex];
        IInterfaceTable *interfaceTable = interfaceInfo->node->interfaceTable;
        GenericRoutingTable *routingTable = interfaceInfo->node->routingTable;

        // count how many (non-loopback) interfaces this node has
        int numInterfaces = 0;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
            if (!interfaceTable->getInterface(i)->isLoopback())
                numInterfaces++;


        if (numInterfaces > 1 && routingTable && routingTable->isForwardingEnabled()) {
            // node has at least one more interface, supposedly connecting to another link
            if (gatewayInterfaceInfo)
                return NULL; // we already found one gateway, this makes it ambiguous! report "no gateway"
            else
                gatewayInterfaceInfo = interfaceInfo; // remember gateway
        }
    }
    return gatewayInterfaceInfo;
}

GenericNetworkConfigurator::InterfaceInfo *GenericNetworkConfigurator::createInterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(node, linkInfo, ie);
    GenericNetworkProtocolInterfaceData *genericData = ie->getGenericNetworkProtocolData();
    if (genericData)
        interfaceInfo->address = genericData->getAddress();
    node->interfaceInfos.push_back(interfaceInfo);
    return interfaceInfo;
}

GenericNetworkConfigurator::Matcher::Matcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

GenericNetworkConfigurator::Matcher::~Matcher()
{
    for (int i = 0; i < (int)matchers.size(); i++)
        delete matchers[i];
}

bool GenericNetworkConfigurator::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (int i = 0; i < (int)matchers.size(); i++)
        if (matchers[i]->matches(s))
            return true;

    return false;
}

void GenericNetworkConfigurator::dumpTopology(GenericTopology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        EV_INFO << "Node " << node->module->getFullPath() << endl;
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::LinkOut *linkOut = node->getLinkOut(j);
            ASSERT(linkOut->getLocalNode() == node);
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            EV_INFO << "     -> " << remoteNode->module->getFullPath() << " " << linkOut->getWeight() << endl;
        }
        for (int j = 0; j < node->getNumInLinks(); j++) {
            Topology::LinkIn *linkIn = node->getLinkIn(j);
            ASSERT(linkIn->getLocalNode() == node);
            Node *remoteNode = (Node *)linkIn->getRemoteNode();
            EV_INFO << "     <- " << remoteNode->module->getFullPath() << " " << linkIn->getWeight() << endl;
        }
    }
}

void GenericNetworkConfigurator::dumpRoutes(GenericTopology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable) {
            EV_INFO << "Node " << node->module->getFullPath() << endl;
            // TODO: node->routingTable->printRoutingTable();
            if (node->routingTable->getNumMulticastRoutes() > 0)
                ; // TODO: node->routingTable->printMulticastRoutingTable();
        }
    }
}

void GenericNetworkConfigurator::addStaticRoutes(GenericTopology& topology)
{
    long calculateShortestPathsDuration = 0;

    // TODO: it should be configurable (via xml?) which nodes need static routes filled in automatically
    // add static routes for all routing tables
    for (int i = 0; i < topology.getNumNodes(); i++) {
        // extract source
        Node *sourceNode = (Node *)topology.getNode(i);
        if (!sourceNode->interfaceTable)
            continue;
        GenericRoutingTable *sourceRoutingTable = sourceNode->routingTable;
        if (!sourceRoutingTable)
            continue;
        //IInterfaceTable *sourceInterfaceTable = sourceNode->interfaceTable;

        // calculate shortest paths from everywhere to sourceNode
        // we are going to use the paths in reverse direction (assuming all links are bidirectional)
        long begin = clock();
        topology.calculateUnweightedSingleShortestPathsTo(sourceNode);
        calculateShortestPathsDuration += clock() - begin;

        // add a route to all destinations in the network
        for (int j = 0; j < topology.getNumNodes(); j++) {
            if (i == j)
                continue;

            // extract destination
            Node *destinationNode = (Node *)topology.getNode(j);
            if (destinationNode->getNumPaths() == 0)
                continue;
            if (!destinationNode->interfaceTable)
                continue;
            //int destinationGateId = destinationNode->getPath(0)->getLocalGateId();
            IInterfaceTable *destinationInterfaceTable = destinationNode->interfaceTable;

            // determine next hop interface
            // find next hop interface (the last IP interface on the path that is not in the source node)
            Node *node = destinationNode;
            Link *link = NULL;
            InterfaceEntry *nextHopInterfaceEntry = NULL;
            while (node != sourceNode) {
                link = (Link *)node->getPath(0);
                if (node->interfaceTable && node != sourceNode && link->sourceInterfaceInfo)
                    nextHopInterfaceEntry = link->sourceInterfaceInfo->interfaceEntry;
                node = (Node *)node->getPath(0)->getRemoteNode();
            }

            // determine source interface
            InterfaceEntry *sourceInterfaceEntry = link->destinationInterfaceInfo->interfaceEntry;

            // add the same routes for all destination interfaces (IP packets are accepted from any interface at the destination)
            for (int j = 0; j < destinationInterfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *destinationInterfaceEntry = destinationInterfaceTable->getInterface(j);
                if (!destinationInterfaceEntry->getGenericNetworkProtocolData())
                    continue;
                L3Address destinationAddress = destinationInterfaceEntry->getGenericNetworkProtocolData()->getAddress();
                if (!destinationInterfaceEntry->isLoopback() && !destinationAddress.isUnspecified() && nextHopInterfaceEntry->getGenericNetworkProtocolData()) {
                    GenericRoute *route = new GenericRoute();
                    L3Address nextHopAddress = nextHopInterfaceEntry->getGenericNetworkProtocolData()->getAddress();
                    route->setSourceType(IRoute::MANUAL);
                    route->setDestination(destinationAddress);
                    route->setInterface(sourceInterfaceEntry);
                    if (nextHopAddress != destinationAddress)
                        route->setNextHop(nextHopAddress);
                    EV_DEBUG << "Adding route " << sourceInterfaceEntry->getFullPath() << " -> " << destinationInterfaceEntry->getFullPath() << " as " << route->info() << endl;
                    sourceRoutingTable->addRoute(route);
                }
            }
        }
    }

    // print some timing information
    printTimeSpentUsingDuration("calculateShortestPaths", calculateShortestPathsDuration);
}

} // namespace inet

