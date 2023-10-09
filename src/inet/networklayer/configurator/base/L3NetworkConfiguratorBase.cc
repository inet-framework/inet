//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"

#include <set>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"

#ifdef INET_WITH_IEEE80211
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#endif

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/common/signal/Interference.h"
#endif

namespace inet {

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
using namespace inet::physicallayer;
#endif

L3NetworkConfiguratorBase::InterfaceInfo::InterfaceInfo(Node *node, LinkInfo *linkInfo, NetworkInterface *networkInterface)
{
    this->node = node;
    this->linkInfo = linkInfo;
    this->networkInterface = networkInterface;
    mtu = -1;
    metric = -1;
    configure = false;
    addStaticRoute = true;
    addDefaultRoute = true;
    addSubnetRoute = true;
}

void L3NetworkConfiguratorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        minLinkWeight = par("minLinkWeight");
        configureIsolatedNetworksSeparatly = par("configureIsolatedNetworksSeparatly").boolValue();
        configuration = par("config");
    }
}

void L3NetworkConfiguratorBase::extractTopology(Topology& topology)
{
    EV_INFO << "Extracting network topology" << endl;
    // extract topology
    topology.extractByProperty("networkNode");
    EV_DEBUG << "Topology contains " << topology.getNumNodes() << " nodes" << endl;

    // print isolated networks information
    std::map<int, std::vector<Node *>> isolatedNetworks;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        int networkId = node->getNetworkId();
        auto networkNodes = isolatedNetworks.find(networkId);
        if (networkNodes == isolatedNetworks.end()) {
            std::vector<Node *> collection = { node };
            isolatedNetworks[networkId] = collection;
        }
        else
            networkNodes->second.push_back(node);
    }
    if (isolatedNetworks.size() == 1)
        EV_DEBUG << "All network nodes belong to a connected network" << endl;
    else
        EV_DEBUG << "There exists " << isolatedNetworks.size() << " isolated networks" << endl;

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        node->module = module;
        node->interfaceTable = findInterfaceTable(node);
        node->routingTable = findRoutingTable(node);
    }

    // extract links and interfaces
    int numLinks = 0;
    std::map<int, NetworkInterface *> interfacesSeen;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                NetworkInterface *networkInterface = interfaceTable->getInterface(j);
                if (!networkInterface->isLoopback() && !containsKey(interfacesSeen, networkInterface->getId())) {
                    if (isBridgeNode(node))
                        createInterfaceInfo(topology, node, nullptr, networkInterface);
                    else {
                        interfacesSeen[networkInterface->getId()] = networkInterface;
                        // create a new network link
                        numLinks++;
                        LinkInfo *linkInfo = new LinkInfo();
                        linkInfo->networkId = node->getNetworkId();
                        topology.linkInfos.push_back(linkInfo);
                        // store interface as belonging to the new network link
                        InterfaceInfo *interfaceInfo = createInterfaceInfo(topology, node, isBridgeNode(node) ? nullptr : linkInfo, networkInterface);
                        linkInfo->interfaceInfos.push_back(interfaceInfo);
                        // visit neighbors (and potentially the whole LAN, recursively)
                        if (networkInterface->isWireless()) {
                            std::vector<Node *> empty;
                            auto wirelessId = getWirelessId(networkInterface);
                            extractWirelessNeighbors(topology, wirelessId.c_str(), linkInfo, interfacesSeen, empty);
                        }
                        else {
                            Topology::Link *linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
                            if (linkOut) {
                                std::vector<Node *> empty;
                                extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, empty);
                            }
                        }
                    }
                }
            }
        }
    }
    EV_DEBUG << "Topology contains " << numLinks << " links" << endl;

    // annotate links with interfaces
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::Link *linkOut = node->getLinkOut(j);
            Link *link = (Link *)linkOut;
            Node *localNode = (Node *)linkOut->getLinkOutLocalNode();
            if (localNode->interfaceTable)
                link->sourceInterfaceInfo = findInterfaceInfo(localNode, localNode->interfaceTable->findInterfaceByNodeOutputGateId(linkOut->getLinkOutLocalGateId()));
            Node *remoteNode = (Node *)linkOut->getLinkOutRemoteNode();
            if (remoteNode->interfaceTable)
                link->destinationInterfaceInfo = findInterfaceInfo(remoteNode, remoteNode->interfaceTable->findInterfaceByNodeInputGateId(linkOut->getLinkOutRemoteGateId()));
        }
    }

    // collect wireless LAN interface infos into a map
    std::map<std::string, std::vector<InterfaceInfo *>> wirelessIdToInterfaceInfosMap;
    for (auto& entry : topology.interfaceInfos) {
        InterfaceInfo *interfaceInfo = entry.second;
        NetworkInterface *networkInterface = interfaceInfo->networkInterface;
        if (!networkInterface->isLoopback() && networkInterface->isWireless()) {
            auto wirelessId = getWirelessId(networkInterface);
            wirelessIdToInterfaceInfosMap[wirelessId].push_back(interfaceInfo);
        }
    }
    EV_DEBUG << "Topology contains " << wirelessIdToInterfaceInfosMap.size() << " wireless LANs" << endl;

    // add extra links between all pairs of wireless interfaces within a LAN (full graph)
    for (auto& entry : wirelessIdToInterfaceInfosMap) {
        std::vector<InterfaceInfo *>& interfaceInfos = entry.second;
        for (size_t i = 0; i < interfaceInfos.size(); i++) {
            InterfaceInfo *interfaceInfoI = interfaceInfos.at(i);
            for (size_t j = i + 1; j < interfaceInfos.size(); j++) {
                // assume bidirectional links
                InterfaceInfo *interfaceInfoJ = interfaceInfos.at(j);
                Link *linkIJ = new Link();
                linkIJ->sourceInterfaceInfo = interfaceInfoI;
                linkIJ->destinationInterfaceInfo = interfaceInfoJ;
                topology.addLink(linkIJ, interfaceInfoI->node, interfaceInfoJ->node);
                Link *linkJI = new Link();
                linkJI->sourceInterfaceInfo = interfaceInfoJ;
                linkJI->destinationInterfaceInfo = interfaceInfoI;
                topology.addLink(linkJI, interfaceInfoJ->node, interfaceInfoI->node);
            }
        }
    }

    // determine gatewayInterfaceInfo for all linkInfos
    for (auto& linkInfo : topology.linkInfos)
        linkInfo->gatewayInterfaceInfo = determineGatewayForLink(linkInfo);
}

void L3NetworkConfiguratorBase::extractWiredNeighbors(Topology& topology, Topology::Link *linkOut, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    Node *node = (Node *)linkOut->getLinkOutRemoteNode();
    int inputGateId = linkOut->getLinkOutRemoteGateId();
    IInterfaceTable *interfaceTable = node->interfaceTable;
    if (!isBridgeNode(node)) {
        NetworkInterface *networkInterface = interfaceTable->findInterfaceByNodeInputGateId(inputGateId);
        if (!networkInterface) {
            // no such interface (node is probably down); we should probably get the information from our (future) internal database
        }
        else if (!containsKey(interfacesSeen, networkInterface->getId())) {
            InterfaceInfo *neighborInterfaceInfo = createInterfaceInfo(topology, node, linkInfo, networkInterface);
            linkInfo->interfaceInfos.push_back(neighborInterfaceInfo);
            interfacesSeen[networkInterface->getId()] = networkInterface;
        }
    }
    else {
        if (!contains(deviceNodesVisited, node))
            extractDeviceNeighbors(topology, node, linkInfo, interfacesSeen, deviceNodesVisited);
    }
}

void L3NetworkConfiguratorBase::extractWirelessNeighbors(Topology& topology, const char *wirelessId, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    for (int nodeIndex = 0; nodeIndex < topology.getNumNodes(); nodeIndex++) {
        Node *node = (Node *)topology.getNode(nodeIndex);
        IInterfaceTable *interfaceTable = node->interfaceTable;
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                NetworkInterface *networkInterface = interfaceTable->getInterface(j);
                if (!networkInterface->isLoopback() && !containsKey(interfacesSeen, networkInterface->getId()) && networkInterface->isWireless()) {
                    if (getWirelessId(networkInterface) == wirelessId) {
                        if (!isBridgeNode(node)) {
                            InterfaceInfo *interfaceInfo = createInterfaceInfo(topology, node, linkInfo, networkInterface);
                            linkInfo->interfaceInfos.push_back(interfaceInfo);
                            interfacesSeen[networkInterface->getId()] = networkInterface;
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

void L3NetworkConfiguratorBase::extractDeviceNeighbors(Topology& topology, Node *node, LinkInfo *linkInfo, std::map<int, NetworkInterface *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    deviceNodesVisited.push_back(node);
    IInterfaceTable *interfaceTable = node->interfaceTable;
    if (interfaceTable) {
        // switch and access point
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            NetworkInterface *networkInterface = interfaceTable->getInterface(i);
            if (!networkInterface->isLoopback() && !containsKey(interfacesSeen, networkInterface->getId())) {
                if (networkInterface->isWireless())
                    extractWirelessNeighbors(topology, getWirelessId(networkInterface).c_str(), linkInfo, interfacesSeen, deviceNodesVisited);
                else {
                    Topology::Link *linkOut = findLinkOut(node, networkInterface->getNodeOutputGateId());
                    if (linkOut)
                        extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, deviceNodesVisited);
                }
            }
        }
    }
    else {
        // hub and bus
        for (int i = 0; i < node->getNumOutLinks(); i++) {
            Topology::Link *linkOut = node->getLinkOut(i);
            extractWiredNeighbors(topology, linkOut, linkInfo, interfacesSeen, deviceNodesVisited);
        }
    }
}

// TODO replace isBridgeNode with isBridgedInterfaces(NetworkInterface *entry1, NetworkInterface *entry2)
// TODO where the two interfaces must be in the same node (meaning they are on the same link)
bool L3NetworkConfiguratorBase::isBridgeNode(Node *node)
{
    return !node->routingTable || !node->interfaceTable;
}

Topology::Link *L3NetworkConfiguratorBase::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLinkOutLocalGateId() == gateId)
            return node->getLinkOut(i);

    return nullptr;
}

L3NetworkConfiguratorBase::InterfaceInfo *L3NetworkConfiguratorBase::findInterfaceInfo(Node *node, NetworkInterface *networkInterface)
{
    if (networkInterface == nullptr)
        return nullptr;
    for (auto& interfaceInfo : node->interfaceInfos)
        if (interfaceInfo->networkInterface == networkInterface)
            return interfaceInfo;

    return nullptr;
}

static double parseCostAttribute(const char *costAttribute)
{
    if (!strncmp(costAttribute, "inf", 3))
        return INFINITY;
    else {
        double cost = atof(costAttribute);
        if (cost <= 0)
            throw cRuntimeError("Cost cannot be less than or equal to zero");
        return cost;
    }
}

double L3NetworkConfiguratorBase::computeNodeWeight(Node *node, const char *metric, cXMLElement *parameters)
{
    const char *costAttribute = parameters->getAttribute("cost");
    if (costAttribute != nullptr)
        return parseCostAttribute(costAttribute);
    else {
        if (node->routingTable && !node->routingTable->isForwardingEnabled())
            return INFINITY;
        else
            return 0;
    }
}

double L3NetworkConfiguratorBase::computeLinkWeight(Link *link, const char *metric, cXMLElement *parameters)
{
    if ((link->sourceInterfaceInfo && link->sourceInterfaceInfo->networkInterface->isWireless()) ||
        (link->destinationInterfaceInfo && link->destinationInterfaceInfo->networkInterface->isWireless()))
        return computeWirelessLinkWeight(link, metric, parameters);
    else
        return computeWiredLinkWeight(link, metric, parameters);
}

double L3NetworkConfiguratorBase::computeWiredLinkWeight(Link *link, const char *metric, cXMLElement *parameters)
{
    const char *costAttribute = parameters->getAttribute("cost");
    if (costAttribute != nullptr)
        return parseCostAttribute(costAttribute);
    else {
        Topology::Link *linkOut = static_cast<Topology::Link *>(static_cast<Topology::Link *>(link));
        if (!strcmp(metric, "hopCount"))
            return 1;
        else if (!strcmp(metric, "delay")) {
            cDatarateChannel *transmissionChannel = dynamic_cast<cDatarateChannel *>(linkOut->getLinkOutLocalGate()->findTransmissionChannel());
            if (transmissionChannel != nullptr)
                return transmissionChannel->getDelay().dbl();
            else
                return minLinkWeight;
        }
        else if (!strcmp(metric, "dataRate")) {
            cChannel *transmissionChannel = linkOut->getLinkOutLocalGate()->findTransmissionChannel();
            if (transmissionChannel != nullptr) {
                double dataRate = transmissionChannel->getNominalDatarate();
                return dataRate != 0 ? 1 / dataRate : minLinkWeight;
            }
            else
                return minLinkWeight;
        }
        else if (!strcmp(metric, "errorRate")) {
            cDatarateChannel *transmissionChannel = dynamic_cast<cDatarateChannel *>(linkOut->getLinkOutLocalGate()->findTransmissionChannel());
            if (transmissionChannel != nullptr) {
                InterfaceInfo *sourceInterfaceInfo = link->sourceInterfaceInfo;
                double bitErrorRate = transmissionChannel->getBitErrorRate();
                double packetErrorRate = 1.0 - pow(1.0 - bitErrorRate, sourceInterfaceInfo->networkInterface->getMtu());
                return minLinkWeight - log(1 - packetErrorRate);
            }
            else
                return minLinkWeight;
        }
        else
            throw cRuntimeError("Unknown metric");
    }
}

double L3NetworkConfiguratorBase::computeWirelessLinkWeight(Link *link, const char *metric, cXMLElement *parameters)
{
    const char *costAttribute = parameters->getAttribute("cost");
    if (costAttribute != nullptr)
        return parseCostAttribute(costAttribute);
    else {
        if (!strcmp(metric, "hopCount"))
            return 1;
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        else if (!strcmp(metric, "delay")) {
            // compute the delay between the two interfaces using a dummy transmission
            const InterfaceInfo *transmitterInterfaceInfo = link->sourceInterfaceInfo;
            const InterfaceInfo *receiverInterfaceInfo = link->destinationInterfaceInfo;
            cModule *transmitterInterfaceModule = transmitterInterfaceInfo->networkInterface;
            cModule *receiverInterfaceModule = receiverInterfaceInfo->networkInterface;
            const IRadio *transmitterRadio = check_and_cast<IRadio *>(transmitterInterfaceModule->getSubmodule("radio"));
            const IRadio *receiverRadio = check_and_cast<IRadio *>(receiverInterfaceModule->getSubmodule("radio"));
            Packet *macFrame = new Packet();
            auto byteCountChunk = makeShared<ByteCountChunk>(B(transmitterInterfaceInfo->networkInterface->getMtu()));
            macFrame->insertAtBack(byteCountChunk);

            // KLUDGE the frame must contain the PHY header to create a transmission
            macFrame->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ackingMac);
            check_and_cast<const Radio *>(transmitterRadio)->encapsulate(macFrame);

            const IRadioMedium *radioMedium = receiverRadio->getMedium();
            const ITransmission *transmission = transmitterRadio->getTransmitter()->createTransmission(transmitterRadio, macFrame, simTime());
            const IArrival *arrival = radioMedium->getPropagation()->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            return arrival->getStartPropagationTime().dbl();
        }
        else if (!strcmp(metric, "dataRate")) {
            cModule *transmitterInterfaceModule = link->sourceInterfaceInfo->networkInterface;
            IRadio *transmitterRadio = check_and_cast<IRadio *>(transmitterInterfaceModule->getSubmodule("radio"));
            const FlatTransmitterBase *transmitter = dynamic_cast<const FlatTransmitterBase *>(transmitterRadio->getTransmitter());
            double dataRate = transmitter ? transmitter->getBitrate().get() : 0;
            return dataRate != 0 ? 1 / dataRate : minLinkWeight;
        }
        else if (!strcmp(metric, "errorRate")) {
            // compute the packet error rate between the two interfaces using a dummy transmission
            const InterfaceInfo *transmitterInterfaceInfo = link->sourceInterfaceInfo;
            const InterfaceInfo *receiverInterfaceInfo = link->destinationInterfaceInfo;
            cModule *transmitterInterfaceModule = transmitterInterfaceInfo->networkInterface;
            cModule *receiverInterfaceModule = receiverInterfaceInfo->networkInterface;
            const IRadio *transmitterRadio = check_and_cast<IRadio *>(transmitterInterfaceModule->getSubmodule("radio"));
            const IRadio *receiverRadio = check_and_cast<IRadio *>(receiverInterfaceModule->getSubmodule("radio"));
            const IRadioMedium *medium = receiverRadio->getMedium();
            Packet *transmittedFrame = new Packet();
            auto byteCountChunk = makeShared<ByteCountChunk>(B(transmitterInterfaceInfo->networkInterface->getMtu()));
            transmittedFrame->insertAtBack(byteCountChunk);

            // KLUDGE the frame must contain the PHY header to create a transmission
            transmittedFrame->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ackingMac);
            check_and_cast<const Radio *>(transmitterRadio)->encapsulate(transmittedFrame);

            const ITransmission *transmission = transmitterRadio->getTransmitter()->createTransmission(transmitterRadio, transmittedFrame, simTime());
            const IArrival *arrival = medium->getPropagation()->computeArrival(transmission, receiverRadio->getAntenna()->getMobility());
            const IListening *listening = receiverRadio->getReceiver()->createListening(receiverRadio, arrival->getStartTime(), arrival->getEndTime(), arrival->getStartPosition(), arrival->getEndPosition());
            const INoise *noise = medium->getBackgroundNoise() != nullptr ? medium->getBackgroundNoise()->computeNoise(listening) : nullptr;
            const IReception *reception = medium->getAnalogModel()->computeReception(receiverRadio, transmission, arrival);
            const IInterference *interference = new Interference(noise, new std::vector<const IReception *>());
            const ISnir *snir = medium->getAnalogModel()->computeSNIR(reception, noise);
            const IReceiver *receiver = receiverRadio->getReceiver();
            bool isReceptionPossible = receiver->computeIsReceptionPossible(listening, reception, IRadioSignal::SIGNAL_PART_WHOLE);
            double packetErrorRate;
            if (!isReceptionPossible)
                packetErrorRate = 1;
            else {
                const IReceptionDecision *receptionDecision = new ReceptionDecision(reception, IRadioSignal::SIGNAL_PART_WHOLE, isReceptionPossible, true, true);
                const std::vector<const IReceptionDecision *> *receptionDecisions = new std::vector<const IReceptionDecision *> { receptionDecision };
                const IReceptionResult *receptionResult = receiver->computeReceptionResult(listening, reception, interference, snir, receptionDecisions);
                Packet *receivedFrame = const_cast<Packet *>(receptionResult->getPacket());
                packetErrorRate = receivedFrame->getTag<ErrorRateInd>()->getPacketErrorRate();
                delete receptionResult;
                delete receptionDecision;
            }
            delete snir;
            delete interference;
            delete reception;
            delete listening;
            delete arrival;
            delete transmission;
            delete transmittedFrame;
            // we want to have a maximum PER product along the path,
            // but still minimize the hop count if the PER is negligible
            return minLinkWeight - log(1 - packetErrorRate);
        }
#endif
        else
            throw cRuntimeError("Unknown metric");
    }
}

/**
 * If this function returns the same string for two wireless interfaces, they
 * will be regarded as being in the same wireless network. (The actual value
 * of the string doesn't count.)
 */
std::string L3NetworkConfiguratorBase::getWirelessId(NetworkInterface *networkInterface)
{
    // use the configuration
    cModule *hostModule = networkInterface->getInterfaceTable()->getHostModule();
    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
    cXMLElementList wirelessElements = configuration->getChildrenByTagName("wireless");
    for (auto& wirelessElement : wirelessElements) {
        const char *hostAttr = wirelessElement->getAttribute("hosts"); // "host* router[0..3]"
        const char *interfaceAttr = wirelessElement->getAttribute("interfaces"); // i.e. interface names, like "eth* ppp0"
        try {
            // parse host/interface expressions
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);

            // Note: "hosts", "interfaces" must ALL match on the interface for the rule to apply
            if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                (interfaceMatcher.matchesAny() || interfaceMatcher.matches(networkInterface->getInterfaceName())))
            {
                const char *idAttr = wirelessElement->getAttribute("id"); // identifier of wireless connection
                return idAttr ? idAttr : wirelessElement->getSourceLocation();
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <wireless> element at %s: %s", wirelessElement->getSourceLocation(), e.what());
        }
    }
#if defined(INET_WITH_IEEE80211) || defined(INET_WITH_PHYSICALLAYERWIRELESSCOMMON)
    cModule *interfaceModule = networkInterface;
#ifdef INET_WITH_IEEE80211
    if (auto mibModule = dynamic_cast<ieee80211::Ieee80211Mib *>(interfaceModule->getSubmodule("mib"))) {
        auto ssid = mibModule->bssData.ssid;
        if (ssid.length() != 0)
            return ssid;
    }
    cModule *mgmtModule = interfaceModule->getSubmodule("mgmt");
    if (mgmtModule != nullptr && mgmtModule->hasPar("ssid")) {
        const char *value = mgmtModule->par("ssid");
        if (*value)
            return value;
    }
    cModule *agentModule = interfaceModule->getSubmodule("agent");
    if (agentModule != nullptr && agentModule->hasPar("defaultSsid")) {
        const char *value = agentModule->par("defaultSsid");
        if (*value)
            return value;
    }
#endif // INET_WITH_IEEE80211
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
    cModule *radioModule = interfaceModule->getSubmodule("radio");
    const IRadio *radio = dynamic_cast<const IRadio *>(radioModule);
    if (radio != nullptr) {
        const cModule *mediumModule = dynamic_cast<const cModule *>(radio->getMedium());
        if (mediumModule != nullptr)
            return mediumModule->getFullName();
    }
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#endif // defined(INET_WITH_IEEE80211) || defined(INET_WITH_PHYSICALLAYERWIRELESSCOMMON)

    // default: put all such wireless interfaces on the same LAN
    return "SSID";
}

/**
 * If this link has exactly one node that connects to other links as well, we can assume
 * it is a "gateway" and return that (we'll use it in routing); otherwise return nullptr.
 */
L3NetworkConfiguratorBase::InterfaceInfo *L3NetworkConfiguratorBase::determineGatewayForLink(LinkInfo *linkInfo)
{
    InterfaceInfo *gatewayInterfaceInfo = nullptr;
    for (auto& interfaceInfo : linkInfo->interfaceInfos) {
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
                return nullptr; // we already found one gateway, this makes it ambiguous! report "no gateway"
            else
                gatewayInterfaceInfo = interfaceInfo; // remember gateway
        }
    }
    return gatewayInterfaceInfo;
}

IInterfaceTable *L3NetworkConfiguratorBase::findInterfaceTable(Node *node)
{
    return L3AddressResolver::findInterfaceTableOf(node->module);
}

IRoutingTable *L3NetworkConfiguratorBase::findRoutingTable(Node *node)
{
    return nullptr;
}

L3NetworkConfiguratorBase::InterfaceInfo *L3NetworkConfiguratorBase::createInterfaceInfo(Topology& topology, Node *node, LinkInfo *linkInfo, NetworkInterface *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(node, linkInfo, ie);
    node->interfaceInfos.push_back(interfaceInfo);
    topology.interfaceInfos[ie->getId()] = interfaceInfo;
    return interfaceInfo;
}

L3NetworkConfiguratorBase::Matcher::Matcher(const char *pattern)
{
    matchesany = opp_isempty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

L3NetworkConfiguratorBase::Matcher::~Matcher()
{
    for (auto& matcher : matchers)
        delete matcher;
}

bool L3NetworkConfiguratorBase::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (auto& matcher : matchers)
        if (matcher->matches(s))
            return true;

    return false;
}

L3NetworkConfiguratorBase::InterfaceMatcher::InterfaceMatcher(const char *pattern)
{
    matchesany = opp_isempty(pattern);
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

L3NetworkConfiguratorBase::InterfaceMatcher::~InterfaceMatcher()
{
    for (auto& nameMatcher : nameMatchers)
        delete nameMatcher;
    for (auto& towardsMatcher : towardsMatchers)
        delete towardsMatcher;
}

bool L3NetworkConfiguratorBase::InterfaceMatcher::matches(InterfaceInfo *interfaceInfo)
{
    if (matchesany)
        return true;

    const char *interfaceName = interfaceInfo->networkInterface->getInterfaceName();
    for (auto& nameMatcher : nameMatchers)
        if (nameMatcher->matches(interfaceName))
            return true;

    LinkInfo *linkInfo = interfaceInfo->linkInfo;
    cModule *ownerModule = interfaceInfo->networkInterface->getInterfaceTable()->getHostModule();
    for (auto& candidateInfo : linkInfo->interfaceInfos) {
        cModule *candidateModule = candidateInfo->networkInterface->getInterfaceTable()->getHostModule();
        if (candidateModule == ownerModule)
            continue;
        std::string candidateFullPath = candidateModule->getFullPath();
        std::string candidateShortenedFullPath = candidateFullPath.substr(candidateFullPath.find('.') + 1);
        for (auto& towardsMatcher : towardsMatchers)
            if (towardsMatcher->matches(candidateShortenedFullPath.c_str()) ||
                towardsMatcher->matches(candidateFullPath.c_str()))
                return true;

    }
    return false;
}

void L3NetworkConfiguratorBase::dumpTopology(Topology& topology)
{
    EV_INFO << "Printing connections of " << topology.getNumNodes() << " nodes" << endl;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        EV_INFO << "Node " << node->module->getFullPath() << endl;
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            Topology::Link *linkOut = node->getLinkOut(j);
            ASSERT(linkOut->getLinkOutLocalNode() == node);
            Node *remoteNode = (Node *)linkOut->getLinkOutRemoteNode();
            EV_INFO << "     -> " << remoteNode->module->getFullPath() << endl;
        }
        for (int j = 0; j < node->getNumInLinks(); j++) {
            Topology::Link *linkIn = node->getLinkIn(j);
            ASSERT(linkIn->getLinkInLocalNode() == node);
            Node *remoteNode = (Node *)linkIn->getLinkInRemoteNode();
            EV_INFO << "     <- " << remoteNode->module->getFullPath() << endl;
        }
    }
}

} // namespace inet

