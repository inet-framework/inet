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
#include "inet/common/ModuleAccess.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/networklayer/common/InterfaceEntry.h"

#ifdef WITH_RADIO
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/IRadioMedium.h"
#include "inet/physicallayer/base/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/base/NarrowbandReceiverBase.h"
#endif

namespace inet {

#ifdef WITH_RADIO
using namespace inet::physicallayer;
#endif

NetworkConfiguratorBase::InterfaceInfo::InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry)
{
    this->node = node;
    this->linkInfo = linkInfo;
    this->interfaceEntry = interfaceEntry;
    mtu = -1;
    metric = -1;
    configure = false;
    addStaticRoute = true;
    addDefaultRoute = true;
    addSubnetRoute = true;
}

void NetworkConfiguratorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        linkWeightMode = par("linkWeightMode");
        defaultLinkWeight = par("defaultLinkWeight");
        minLinkWeight = par("minLinkWeight");
        configuration = par("config");
    }
}

void NetworkConfiguratorBase::extractTopology(Topology& topology)
{
    // extract topology
    topology.extractByProperty("node");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        node->module = module;
        node->interfaceTable = findInterfaceTable(node);
        node->routingTable = findRoutingTable(node);
        node->setWeight(computeNodeWeight(node));
    }

    // extract links and interfaces
    std::set<InterfaceEntry *> interfacesSeen;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable && !isBridgeNode(node)) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *interfaceEntry = interfaceTable->getInterface(j);
                if (!interfaceEntry->isLoopback() && interfacesSeen.count(interfaceEntry) == 0) {
                    // create a new network link
                    LinkInfo *linkInfo = new LinkInfo();
                    topology.linkInfos.push_back(linkInfo);

                    // store interface as belonging to the new network link
                    InterfaceInfo *interfaceInfo = createInterfaceInfo(topology, node, linkInfo, interfaceEntry);
                    linkInfo->interfaceInfos.push_back(interfaceInfo);
                    interfacesSeen.insert(interfaceEntry);

                    // visit neighbors (and potentially the whole LAN, recursively)
                    if (isWirelessInterface(interfaceEntry)) {
                        std::vector<Node *> empty;
                        const char *wirelessId = getWirelessId(interfaceEntry);
                        extractWirelessNeighbors(topology, wirelessId, linkInfo, interfacesSeen, empty);
                    }
                    else {
                        Topology::LinkOut *linkOut = findLinkOut(node, interfaceEntry->getNodeOutputGateId());
                        if (linkOut) {
                            std::vector<Node *> empty;
                            extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, empty);
                        }
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

    // collect wireless LAN interface infos into a map
    std::map<std::string, std::vector<InterfaceInfo *> > wirelessIdToInterfaceInfosMap;
    for (int i = 0; i < (int)topology.linkInfos.size(); i++) {
        LinkInfo *linkInfo = topology.linkInfos.at(i);
        for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++) {
            InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos.at(j);
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            if (!interfaceEntry->isLoopback() && isWirelessInterface(interfaceEntry)) {
                const char *wirelessId = getWirelessId(interfaceEntry);
                wirelessIdToInterfaceInfosMap[wirelessId].push_back(interfaceInfo);
            }
        }
    }

    // add extra links between all pairs of wireless interfaces within a LAN (full graph)
    for (auto it = wirelessIdToInterfaceInfosMap.begin(); it != wirelessIdToInterfaceInfosMap.end(); it++) {
        std::vector<InterfaceInfo *>& interfaceInfos = it->second;
        for (int i = 0; i < (int)interfaceInfos.size(); i++) {
            InterfaceInfo *interfaceInfoI = interfaceInfos.at(i);
            for (int j = i + 1; j < (int)interfaceInfos.size(); j++) {
                // assume bidirectional links
                InterfaceInfo *interfaceInfoJ = interfaceInfos.at(j);
                Link *linkIJ = new Link();
                linkIJ->sourceInterfaceInfo = interfaceInfoI;
                linkIJ->destinationInterfaceInfo = interfaceInfoJ;
                linkIJ->setWeight(computeWirelessLinkWeight(linkIJ));
                topology.addLink(linkIJ, interfaceInfoI->node, interfaceInfoJ->node);
                Link *linkJI = new Link();
                linkJI->sourceInterfaceInfo = interfaceInfoJ;
                linkJI->destinationInterfaceInfo = interfaceInfoI;
                linkJI->setWeight(computeWirelessLinkWeight(linkJI));
                topology.addLink(linkJI, interfaceInfoJ->node, interfaceInfoI->node);
            }
        }
    }

    // determine gatewayInterfaceInfo for all linkInfos
    for (int linkIndex = 0; linkIndex < (int)topology.linkInfos.size(); linkIndex++) {
        LinkInfo *linkInfo = topology.linkInfos[linkIndex];
        linkInfo->gatewayInterfaceInfo = determineGatewayForLink(linkInfo);
    }
}

void NetworkConfiguratorBase::extractWiredNeighbors(Topology& topology, Topology::LinkOut *linkOut, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    linkOut->setWeight(computeWiredLinkWeight(static_cast<Link *>(static_cast<Topology::Link *>(linkOut))));
    Node *node = (Node *)linkOut->getRemoteNode();
    int inputGateId = linkOut->getRemoteGateId();
    IInterfaceTable *interfaceTable = node->interfaceTable;
    if (!isBridgeNode(node)) {
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceByNodeInputGateId(inputGateId);
        if (!interfaceEntry) {
            // no such interface (node is probably down); we should probably get the information from our (future) internal database
        }
        else if (interfacesSeen.count(interfaceEntry) == 0) {
            InterfaceInfo *neighborInterfaceInfo = createInterfaceInfo(topology, node, linkInfo, interfaceEntry);
            linkInfo->interfaceInfos.push_back(neighborInterfaceInfo);
            interfacesSeen.insert(interfaceEntry);
        }
    }
    else {
        if (!contains(deviceNodesVisited, node))
            extractDeviceNeighbors(topology, node, linkInfo, interfacesSeen, deviceNodesVisited);
    }
}

void NetworkConfiguratorBase::extractWirelessNeighbors(Topology& topology, const char *wirelessId, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    for (int nodeIndex = 0; nodeIndex < topology.getNumNodes(); nodeIndex++) {
        Node *node = (Node *)topology.getNode(nodeIndex);
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *interfaceEntry = interfaceTable->getInterface(j);
                if (!interfaceEntry->isLoopback() && interfacesSeen.count(interfaceEntry) == 0 && isWirelessInterface(interfaceEntry)) {
                    if (!strcmp(getWirelessId(interfaceEntry), wirelessId)) {
                        if (!isBridgeNode(node)) {
                            InterfaceInfo *interfaceInfo = createInterfaceInfo(topology, node, linkInfo, interfaceEntry);
                            linkInfo->interfaceInfos.push_back(interfaceInfo);
                            interfacesSeen.insert(interfaceEntry);
                        }
                        else {
                            if (!contains(deviceNodesVisited, node))
                                extractDeviceNeighbors(topology, node, linkInfo, interfacesSeen, deviceNodesVisited);
                        }
                    }
                }
            }
        }
    }
}

void NetworkConfiguratorBase::extractDeviceNeighbors(Topology& topology, Node *node, LinkInfo *linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    deviceNodesVisited.push_back(node);
    IInterfaceTable *interfaceTable = node->interfaceTable;
    if (interfaceTable) {
        // switch and access point
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (!interfaceEntry->isLoopback() && interfacesSeen.count(interfaceEntry) == 0) {
                if (isWirelessInterface(interfaceEntry))
                    extractWirelessNeighbors(topology, getWirelessId(interfaceEntry), linkInfo, interfacesSeen, deviceNodesVisited);
                else {
                    Topology::LinkOut *linkOut = findLinkOut(node, interfaceEntry->getNodeOutputGateId());
                    if (linkOut)
                        extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, deviceNodesVisited);
                }
            }
        }
    }
    else {
        // hub and bus
        for (int i = 0; i < node->getNumOutLinks(); i++) {
            Topology::LinkOut *linkOut = node->getLinkOut(i);
            extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, deviceNodesVisited);
        }
    }
}

// TODO: replace isBridgeNode with isBridgedInterfaces(InterfaceEntry *entry1, InterfaceEntry *entry2)
// TODO: where the two interfaces must be in the same node (meaning they are on the same link)
bool NetworkConfiguratorBase::isBridgeNode(Node *node)
{
    return !node->routingTable || !node->interfaceTable;
}

bool NetworkConfiguratorBase::isWirelessInterface(InterfaceEntry *interfaceEntry)
{
    return !strncmp(interfaceEntry->getName(), "wlan", 4);
}

Topology::LinkOut *NetworkConfiguratorBase::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);

    return NULL;
}

NetworkConfiguratorBase::InterfaceInfo *NetworkConfiguratorBase::findInterfaceInfo(Node *node, InterfaceEntry *interfaceEntry)
{
    for (int i = 0; i < (int)node->interfaceInfos.size(); i++)
        if (node->interfaceInfos.at(i)->interfaceEntry == interfaceEntry)
            return node->interfaceInfos.at(i);

    return NULL;
}

double NetworkConfiguratorBase::computeNodeWeight(Node *node)
{
    if (node->routingTable && !node->routingTable->isForwardingEnabled())
        return DBL_MAX;
    else
        return 0;
}

double NetworkConfiguratorBase::computeWiredLinkWeight(Link *link)
{
    Topology::LinkOut *linkOut = static_cast<Topology::LinkOut *>(static_cast<Topology::Link *>(link));
    if (!strcmp(linkWeightMode, "constant"))
        return defaultLinkWeight;
    else if (!strcmp(linkWeightMode, "dataRate")) {
        cChannel *transmissionChannel = linkOut->getLocalGate()->getTransmissionChannel();
        if (transmissionChannel) {
            double dataRate = transmissionChannel->getNominalDatarate();
            return dataRate != 0 ? 1 / dataRate : defaultLinkWeight;
        }
        else
            return defaultLinkWeight;
    }
    else if (!strcmp(linkWeightMode, "errorRate")) {
        cDatarateChannel *transmissionChannel = dynamic_cast<cDatarateChannel *>(linkOut->getLocalGate()->getTransmissionChannel());
        if (!transmissionChannel) {
            InterfaceInfo *sourceInterfaceInfo = link->sourceInterfaceInfo;
            double bitErrorRate = transmissionChannel->getBitErrorRate();
            double packetErrorRate = 1.0 - pow(1.0 - bitErrorRate, sourceInterfaceInfo->interfaceEntry->getMTU());
            return defaultLinkWeight - log(1 - packetErrorRate);
        }
        else
            return defaultLinkWeight;
    }
    else
        throw cRuntimeError("Unknown linkWeightMode");
}

double NetworkConfiguratorBase::computeWirelessLinkWeight(Link *link)
{
    if (!strcmp(linkWeightMode, "constant"))
        return defaultLinkWeight;
    else if (!strcmp(linkWeightMode, "dataRate")) {
#ifdef WITH_RADIO
        // compute the packet error rate between the two interfaces using a dummy transmission
        cModule *transmitterInterfaceModule = link->sourceInterfaceInfo->interfaceEntry->getInterfaceModule();
        IRadio *transmitterRadio = check_and_cast<IRadio *>(transmitterInterfaceModule->getSubmodule("radio"));
        const NarrowbandTransmitterBase *transmitter = dynamic_cast<const NarrowbandTransmitterBase *>(transmitterRadio->getTransmitter());
        double dataRate = transmitter ? transmitter->getBitrate().get() : 0;
        return dataRate != 0 ? 1 / dataRate : defaultLinkWeight;
#else
        throw cRuntimeError("Radio feature is disabled");
#endif
    }
    else if (!strcmp(linkWeightMode, "errorRate")) {
#ifdef WITH_RADIO
        // compute the packet error rate between the two interfaces using a dummy transmission
        InterfaceInfo *transmitterInterfaceInfo = link->sourceInterfaceInfo;
        InterfaceInfo *receiverInterfaceInfo = link->destinationInterfaceInfo;
        cModule *transmitterInterfaceModule = transmitterInterfaceInfo->interfaceEntry->getInterfaceModule();
        cModule *receiverInterfaceModule = receiverInterfaceInfo->interfaceEntry->getInterfaceModule();
        IRadio *transmitterRadio = check_and_cast<IRadio *>(transmitterInterfaceModule->getSubmodule("radio"));
        IRadio *receiverRadio = check_and_cast<IRadio *>(receiverInterfaceModule->getSubmodule("radio"));
        const NarrowbandReceiverBase *receiver = dynamic_cast<const NarrowbandReceiverBase *>(receiverRadio->getReceiver());
        if (receiver) {
            cPacket *macFrame = new cPacket();
            macFrame->setByteLength(transmitterInterfaceInfo->interfaceEntry->getMTU());
            const IRadioMedium *medium = receiverRadio->getMedium();
            const ITransmission *transmission = transmitterRadio->getTransmitter()->createTransmission(transmitterRadio, macFrame, simTime());
            const IArrival *arrival = medium->getPropagation()->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const INoise *noise = medium->getBackgroundNoise()->computeNoise(listening);
            const IReception *reception = medium->getAnalogModel()->computeReception(receiverRadio, transmission, arrival);
            const ISNIR *snir = medium->getAnalogModel()->computeSNIR(reception, noise);
            double packetErrorRate = receiver->getErrorModel()->computePacketErrorRate(snir);
            delete snir;
            delete reception;
            delete noise;
            delete listening;
            delete arrival;
            delete transmission;
            // we want to have a maximum PER product along the path,
            // but still minimize the hop count if the PER is negligible
            return minLinkWeight - log(1 - packetErrorRate);
        }
        return defaultLinkWeight;
#else
        throw cRuntimeError("Radio feature is disabled");
#endif
    }
    else
        throw cRuntimeError("Unknown linkWeightMode");
}

/**
 * If this function returns the same string for two wireless interfaces, they
 * will be regarded as being in the same wireless network. (The actual value
 * of the string doesn't count.)
 */
const char *NetworkConfiguratorBase::getWirelessId(InterfaceEntry *interfaceEntry)
{
    // use the configuration
    cModule *hostModule = interfaceEntry->getInterfaceTable()->getHostModule();
    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
    cXMLElementList wirelessElements = configuration->getChildrenByTagName("wireless");
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
    cModule *mgmtModule = module->getSubmodule("mgmt");
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
NetworkConfiguratorBase::InterfaceInfo *NetworkConfiguratorBase::determineGatewayForLink(LinkInfo *linkInfo)
{
    InterfaceInfo *gatewayInterfaceInfo = NULL;
    for (int interfaceIndex = 0; interfaceIndex < (int)linkInfo->interfaceInfos.size(); interfaceIndex++) {
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[interfaceIndex];
        IInterfaceTable *interfaceTable = interfaceInfo->node->interfaceTable;
        IRoutingTable *routingTable = interfaceInfo->node->routingTable;

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

IInterfaceTable *NetworkConfiguratorBase::findInterfaceTable(Node *node)
{
    return L3AddressResolver::findInterfaceTableOf(node->module);
}

IRoutingTable *NetworkConfiguratorBase::findRoutingTable(Node *node)
{
    return NULL;
}

NetworkConfiguratorBase::InterfaceInfo *NetworkConfiguratorBase::createInterfaceInfo(Topology& topology, Node *node, LinkInfo *linkInfo, InterfaceEntry *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(node, linkInfo, ie);
    node->interfaceInfos.push_back(interfaceInfo);
    topology.interfaceInfos[ie] = interfaceInfo;
    return interfaceInfo;
}

NetworkConfiguratorBase::Matcher::Matcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

NetworkConfiguratorBase::Matcher::~Matcher()
{
    for (int i = 0; i < (int)matchers.size(); i++)
        delete matchers[i];
}

bool NetworkConfiguratorBase::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (int i = 0; i < (int)matchers.size(); i++)
        if (matchers[i]->matches(s))
            return true;

    return false;
}

NetworkConfiguratorBase::InterfaceMatcher::InterfaceMatcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        if (*token == '>')
            towardsMatchers.push_back(new inet::PatternMatcher(token + 1, true, true, true));
        else
            nameMatchers.push_back(new inet::PatternMatcher(token, true, true, true));
    }
}

NetworkConfiguratorBase::InterfaceMatcher::~InterfaceMatcher()
{
    for (int i = 0; i < (int)nameMatchers.size(); i++)
        delete nameMatchers[i];
    for (int i = 0; i < (int)towardsMatchers.size(); i++)
        delete towardsMatchers[i];
}

bool NetworkConfiguratorBase::InterfaceMatcher::matches(InterfaceInfo *interfaceInfo)
{
    if (matchesany)
        return true;

    const char *interfaceName = interfaceInfo->interfaceEntry->getName();
    for (int i = 0; i < (int)nameMatchers.size(); i++)
        if (nameMatchers[i]->matches(interfaceName))
            return true;


    LinkInfo *linkInfo = interfaceInfo->linkInfo;
    cModule *ownerModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
    for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++) {
        InterfaceInfo *candidateInfo = linkInfo->interfaceInfos[i];
        cModule *candidateModule = candidateInfo->interfaceEntry->getInterfaceTable()->getHostModule();
        if (candidateModule == ownerModule)
            continue;
        std::string candidateFullPath = candidateModule->getFullPath();
        std::string candidateShortenedFullPath = candidateFullPath.substr(candidateFullPath.find('.') + 1);
        for (int j = 0; j < (int)towardsMatchers.size(); j++)
            if (towardsMatchers[j]->matches(candidateShortenedFullPath.c_str()) ||
                towardsMatchers[j]->matches(candidateFullPath.c_str()))
                return true;

    }
    return false;
}

void NetworkConfiguratorBase::dumpTopology(Topology& topology)
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

} // namespace inet

