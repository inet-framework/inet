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
#include "stlutils.h"
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "IPvXAddressResolver.h"
#include "IPv4NetworkConfigurator.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "PatternMatcher.h"
#include "ModuleAccess.h"

Define_Module(IPv4NetworkConfigurator);

#define ADDRLEN_BITS 32

inline bool isEmpty(const char *s) {return !s || !s[0];}
inline bool isNotEmpty(const char *s) {return s && s[0];}

static void printTimeSpentUsingDuration(const char *name, long duration)
{
    EV_INFO << "Time spent in IPv4NetworkConfigurator::" << name << ": " << ((double)duration / CLOCKS_PER_SEC) << "s" << endl;
}

static void printElapsedTime(const char *name, long startTime)
{
    printTimeSpentUsingDuration(name, clock() - startTime);
}

#define T(CODE)  {long startTime=clock(); CODE; printElapsedTime(#CODE, startTime);}

void IPv4NetworkConfigurator::initialize(int stage)
{
    if (stage == 2)
    {
        long initializeStartTime = clock();

        IPv4Topology topology;

        // cache parameters used often
        addSubnetRoutesParameter = par("addSubnetRoutes");
        addDefaultRoutesParameter = par("addDefaultRoutes");
        optimizeRoutesParameter = par("optimizeRoutes");
        assignDisjunctSubnetAddressesParameter = par("assignDisjunctSubnetAddresses");

        // extract topology into the IPv4Topology object, then fill in a LinkInfo[] vector
        T(extractTopology(topology));

        // dump the result if requested
        if (par("dumpTopology").boolValue())
            T(dumpTopology(topology));

        // read the configuration from XML; it will serve as input for address assignment
        T(readAddressConfiguration(par("config").xmlValue(), topology));

        // assign addresses to IPv4 nodes
        if (par("assignAddresses").boolValue())
            T(assignAddresses(topology));

        // read and configure multicast groups from the XML configuration
        T(addMulticastGroups(par("config").xmlValue(), topology));

        // print unicast and multicast addresses and other interface data to module output
        if (par("dumpAddresses").boolValue())
            T(dumpAddresses(topology));

        // read and configure manual routes from the XML configuration
        T(addManualRoutes(par("config").xmlValue(), topology));

        // read and configure manual multicast routes from the XML configuration
        T(addManualMulticastRoutes(par("config").xmlValue(), topology));

        // calculate shortest paths, and add corresponding static routes
        if (par("addStaticRoutes").boolValue())
            T(addStaticRoutes(topology));

        // dump routes to module output
        if (par("dumpRoutes").boolValue())
            T(dumpRoutes(topology));

        if (!isEmpty(par("dumpConfig")))
            T(dumpConfig(topology));

        printElapsedTime("initialize", initializeStartTime);
    }
}

#undef T

void IPv4NetworkConfigurator::extractTopology(IPv4Topology& topology)
{
    // TODO: the current topology discovery implementation doesn't support heterogeneous wired and wireless links
    extractWiredTopology(topology);
    extractWirelessTopology(topology);

    // determine gatewayInterfaceInfo for all linkInfos
    for (int linkIndex = 0; linkIndex < (int)topology.linkInfos.size(); linkIndex++)
    {
        LinkInfo *linkInfo = topology.linkInfos[linkIndex];
        linkInfo->gatewayInterfaceInfo = determineGatewayForLink(linkInfo);
    }
}

bool IPv4NetworkConfigurator::isWirelessInterface(InterfaceEntry *interfaceEntry)
{
    return !strncmp(interfaceEntry->getName(), "wlan", 4);
}

Topology::LinkOut *IPv4NetworkConfigurator::findLinkOut(Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return NULL;
}

IPv4NetworkConfigurator::InterfaceInfo *IPv4NetworkConfigurator::findInterfaceInfo(Node *node, InterfaceEntry *ie)
{
    for (int i = 0; i < (int)node->interfaceInfos.size(); i++)
        if (node->interfaceInfos.at(i)->interfaceEntry == ie)
            return node->interfaceInfos.at(i);
    return NULL;
}

void IPv4NetworkConfigurator::extractWiredTopology(IPv4Topology& topology)
{
    // extract topology
    topology.extractByProperty("node");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    // extract nodes, fill in interfaceTable and routingTable members in node
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        node->module = module;
        node->interfaceTable = IPvXAddressResolver().findInterfaceTableOf(module);
        node->routingTable = IPvXAddressResolver().findRoutingTableOf(module);
        if (node->routingTable && !node->routingTable->isIPForwardingEnabled())
            node->setWeight(DBL_MAX);
    }

    // extract links and interfaces
    std::set<InterfaceEntry *> interfacesSeen;
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        cModule *module = node->getModule();
        IInterfaceTable *interfaceTable = IPvXAddressResolver().findInterfaceTableOf(module);
        if (interfaceTable)
        {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++)
            {
                InterfaceEntry *ie = interfaceTable->getInterface(j);
                if (!ie->isLoopback() && interfacesSeen.count(ie) == 0 && !isWirelessInterface(ie))
                {
                    // create a new network link
                    LinkInfo *linkInfo = new LinkInfo(false);
                    topology.linkInfos.push_back(linkInfo);

                    // store interface as belonging to the new network link
                    InterfaceInfo *interfaceInfo = createInterfaceInfo(node, linkInfo, ie);
                    linkInfo->interfaceInfos.push_back(interfaceInfo);
                    interfacesSeen.insert(ie);

                    // visit neighbor (and potentially the whole LAN, recursively)
                    Topology::LinkOut *linkOut = findLinkOut((Node *)topology.getNode(i), ie->getNodeOutputGateId());
                    if (linkOut)
                    {
                        std::vector<Node *> empty;
                        extractWiredNeighbors(linkOut, linkInfo, interfacesSeen, empty);
                    }
                }
            }
        }
    }

    // annotate links with interfaces
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumOutLinks(); j++)
        {
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

void IPv4NetworkConfigurator::extractWiredNeighbors(Topology::LinkOut *linkOut, LinkInfo* linkInfo, std::set<InterfaceEntry *>& interfacesSeen, std::vector<Node *>& deviceNodesVisited)
{
    cChannel *transmissionChannel = linkOut->getLocalGate()->getTransmissionChannel();
    if (transmissionChannel)
        linkOut->setWeight(getChannelWeight(transmissionChannel));

    Node *neighborNode = (Node *)linkOut->getRemoteNode();
    cModule *neighborModule = neighborNode->getModule();
    int neighborInputGateId = linkOut->getRemoteGateId();
    IInterfaceTable *neighborInterfaceTable = IPvXAddressResolver().findInterfaceTableOf(neighborModule);
    if (neighborInterfaceTable)
    {
        // neighbor is a host or router, just add the interface
        InterfaceEntry *neighborInterfaceEntry = neighborInterfaceTable->getInterfaceByNodeInputGateId(neighborInputGateId);
        if (interfacesSeen.count(neighborInterfaceEntry) == 0)
        {
            InterfaceInfo *neighborInterfaceInfo = createInterfaceInfo(neighborNode, linkInfo, neighborInterfaceEntry);
            linkInfo->interfaceInfos.push_back(neighborInterfaceInfo);
            interfacesSeen.insert(neighborInterfaceEntry);
        }
    }
    else
    {
        // assume that neighbor is an L2 or L1 device (bus/hub/switch/bridge/access point/etc); visit all its output links
        Node *deviceNode = (Node *)linkOut->getRemoteNode();
        if (!contains(deviceNodesVisited, deviceNode))
        {
            deviceNodesVisited.push_back(deviceNode);
            for (int i = 0; i < deviceNode->getNumOutLinks(); i++)
            {
                Topology::LinkOut *deviceLinkOut = deviceNode->getLinkOut(i);
                extractWiredNeighbors(deviceLinkOut, linkInfo, interfacesSeen, deviceNodesVisited);
            }
        }
    }
}

double IPv4NetworkConfigurator::getChannelWeight(cChannel *transmissionChannel)
{
    //TODO shouldn't we use interface metric here?
    double datarate = transmissionChannel->getNominalDatarate();
    return datarate > 0 ? 1 / datarate : 1.0;  //TODO why 1.0 if there's no datarate?
}

/**
 * Discover wireless LANs, and create a corresponding LinkInfo for each.
 * We use getWirelessId() on all wireless interfaces to try and determine the
 * WLAN "id" (that's SSID for 802.11) they're on.
 */
void IPv4NetworkConfigurator::extractWirelessTopology(IPv4Topology& topology)
{
    std::map<std::string, LinkInfo *> wirelessIdToLinkInfoMap;  // wireless LANs by name

    // iterate through all wireless interfaces and determine the wireless id.
    for (int nodeIndex = 0; nodeIndex < topology.getNumNodes(); nodeIndex++)
    {
        Node *node = (Node *)topology.getNode(nodeIndex);
        cModule *module = node->getModule();
        IInterfaceTable *interfaceTable = IPvXAddressResolver().findInterfaceTableOf(module);
        if (interfaceTable)
        {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++)
            {
                InterfaceEntry *ie = interfaceTable->getInterface(j);
                if (!ie->isLoopback() && isWirelessInterface(ie))
                {
                    const char *wirelessId = getWirelessId(ie);
                    EV_DEBUG << "Interface " << ie->getFullPath() << " has wireless id " << wirelessId << endl;
                    LinkInfo *linkInfo = wirelessIdToLinkInfoMap[wirelessId];
                    if (!linkInfo)
                    {
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
    for (std::map<std::string, LinkInfo *>::iterator it = wirelessIdToLinkInfoMap.begin(); it != wirelessIdToLinkInfoMap.end(); it++)
    {
        LinkInfo *linkInfo = it->second;
        for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++)
        {
            InterfaceInfo *interfaceInfoI = linkInfo->interfaceInfos.at(i);
            for (int j = i + 1; j < (int)linkInfo->interfaceInfos.size(); j++)
            {
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
const char *IPv4NetworkConfigurator::getWirelessId(InterfaceEntry *interfaceEntry)
{
    // use the configuration
    cModule *hostModule = interfaceEntry->getInterfaceTable()->getHostModule();
    std::string hostFullPath = hostModule->getFullPath();
    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
    cXMLElement *root = par("config").xmlValue();
    cXMLElementList wirelessElements = root->getChildrenByTagName("wireless");
    for (int i = 0; i < (int)wirelessElements.size(); i++)
    {
        cXMLElement *wirelessElement = wirelessElements[i];
        const char *hostAttr = wirelessElement->getAttribute("hosts");  // "host* router[0..3]"
        const char *interfaceAttr = wirelessElement->getAttribute("interfaces"); // i.e. interface names, like "eth* ppp0"
        try
        {
            // parse host/interface expressions
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);

            // Note: "hosts", "interfaces" must ALL match on the interface for the rule to apply
            if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceEntry->getFullName())))
            {
                const char *idAttr = wirelessElement->getAttribute("id");  // identifier of wireless connection
                return idAttr ? idAttr : wirelessElement->getSourceLocation();
            }
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Error in XML <wireless> element at %s: %s", wirelessElement->getSourceLocation(), e.what());
        }
    }

    // if the mgmt submodule within the wireless NIC has an "ssid" or "accessPointAddress" parameter, we can use that
    cModule *module = interfaceEntry->getInterfaceModule();
    if (!module)
        module = hostModule;
    cSimpleModule *mgmtModule = ModuleAccess<cSimpleModule>("mgmt").getIfExists(module);
    if (mgmtModule)
    {
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
IPv4NetworkConfigurator::InterfaceInfo *IPv4NetworkConfigurator::determineGatewayForLink(LinkInfo *linkInfo)
{
    InterfaceInfo *gatewayInterfaceInfo = NULL;
    for (int interfaceIndex = 0; interfaceIndex < (int)linkInfo->interfaceInfos.size(); interfaceIndex++)
    {
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[interfaceIndex];
        IInterfaceTable *interfaceTable = interfaceInfo->node->interfaceTable;
        IRoutingTable *routingTable = interfaceInfo->node->routingTable;

        // count how many (non-loopback) interfaces this node has
        int numInterfaces = 0;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
            if (!interfaceTable->getInterface(i)->isLoopback())
                numInterfaces++;

        if (numInterfaces > 1 && routingTable && routingTable->isIPForwardingEnabled())
        {
            // node has at least one more interface, supposedly connecting to another link
            if (gatewayInterfaceInfo)
                return NULL;  // we already found one gateway, this makes it ambiguous! report "no gateway"
            else
                gatewayInterfaceInfo = interfaceInfo; // remember gateway
        }
    }
    return gatewayInterfaceInfo;
}

/**
 * Returns how many bits are needed to represent count different values.
 */
inline int getRepresentationBitCount(uint32 count)
{
    int bitCount = 0;
    while (((uint32)1 << bitCount) < count)
        bitCount++;
    return bitCount;
}

/**
 * Returns the index of the most significant bit that equals to the given bit value.
 * 0 means the most significant bit.
 */
static int getMostSignificantBitIndex(uint32 value, int bitValue, int defaultIndex)
{
    for (int bitIndex = sizeof(value) * 8 - 1; bitIndex >= 0; bitIndex--)
    {
        uint32 mask = (uint32)1 << bitIndex;
        if ((value & mask) == ((uint32)bitValue << bitIndex))
            return bitIndex;
    }
    return defaultIndex;
}

/**
 * Returns the index of the least significant bit that equals to the given bit value.
 * 0 means the most significant bit.
 */
static int getLeastSignificantBitIndex(uint32 value, int bitValue, int defaultIndex)
{
    for (int bitIndex = 0; bitIndex < ADDRLEN_BITS; bitIndex++)
    {
        uint32 mask = (uint32)1 << bitIndex;
        if ((value & mask) == ((uint32)bitValue << bitIndex))
            return bitIndex;
    }
    return defaultIndex;
}

/**
 * Returns packed bits (subsequent) from value specified by mask (sparse).
 */
static uint32 getPackedBits(uint32 value, uint32 valueMask)
{
    uint32 packedValue = 0;
    int packedValueIndex = 0;
    for (int valueIndex = 0; valueIndex < ADDRLEN_BITS; valueIndex++)
    {
        uint32 valueBitMask = (uint32)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0)
        {
            if ((value & valueBitMask) != 0)
                packedValue |= (uint32)1 << packedValueIndex;
            packedValueIndex++;
        }
    }
    return packedValue;
}

/**
 * Set packed bits (subsequent) in value specified by mask (sparse).
 */
static uint32 setPackedBits(uint32 value, uint32 valueMask, uint32 packedValue)
{
    int packedValueIndex = 0;
    for (int valueIndex = 0; valueIndex < ADDRLEN_BITS; valueIndex++)
    {
        uint32 valueBitMask = (uint32)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0)
        {
            uint32 newValueBitMask = (uint32)1 << packedValueIndex;
            if ((packedValue & newValueBitMask) != 0)
                value |= valueBitMask;
            else
                value &= ~valueBitMask;
            packedValueIndex++;
        }
    }
    return value;
}

/**
 * Returns a subset of the given interfaces that have compatible address and netmask specifications.
 * Determine the merged address and netmask specifications according to the following table.
 * The '?' symbol means the bit is unspecified, the 'X' symbol means the bit is incompatible.
 * | * | 0 | 1 | ? |
 * | 0 | 0 | X | 0 |
 * | 1 | X | 1 | 1 |
 * | ? | 0 | 1 | ? |
 */
void IPv4NetworkConfigurator::collectCompatibleInterfaces(const std::vector<InterfaceInfo *>& interfaces, /*in*/
        std::vector<IPv4NetworkConfigurator::InterfaceInfo *>& compatibleInterfaces, /*out, and the rest too*/
        uint32& mergedAddress, uint32& mergedAddressSpecifiedBits, uint32& mergedAddressIncompatibleBits,
        uint32& mergedNetmask, uint32& mergedNetmaskSpecifiedBits, uint32& mergedNetmaskIncompatibleBits)
{
    ASSERT(compatibleInterfaces.empty());
    mergedAddress = mergedAddressSpecifiedBits = mergedAddressIncompatibleBits = 0;
    mergedNetmask = mergedNetmaskSpecifiedBits = mergedNetmaskIncompatibleBits = 0;

    for (int interfaceIndex = 0; interfaceIndex < (int)interfaces.size(); interfaceIndex++)
    {
        IPv4NetworkConfigurator::InterfaceInfo *candidateInterface = interfaces[interfaceIndex];
        InterfaceEntry *ie = candidateInterface->interfaceEntry;

        // extract candidate interface configuration data
        uint32 candidateAddress = candidateInterface->address;
        uint32 candidateAddressSpecifiedBits = candidateInterface->addressSpecifiedBits;
        uint32 candidateNetmask = candidateInterface->netmask;
        uint32 candidateNetmaskSpecifiedBits = candidateInterface->netmaskSpecifiedBits;
        EV_DEBUG << "Trying to merge " << ie->getFullPath() << " interface with address specification: " << IPv4Address(candidateAddress) << " / " << IPv4Address(candidateAddressSpecifiedBits) << endl;
        EV_DEBUG << "Trying to merge " << ie->getFullPath() << " interface with netmask specification: " << IPv4Address(candidateNetmask) << " / " << IPv4Address(candidateNetmaskSpecifiedBits) << endl;

        // determine merged netmask bits
        uint32 commonNetmaskSpecifiedBits = mergedNetmaskSpecifiedBits & candidateNetmaskSpecifiedBits;
        uint32 newMergedNetmask = mergedNetmask | (candidateNetmask & candidateNetmaskSpecifiedBits);
        uint32 newMergedNetmaskSpecifiedBits = mergedNetmaskSpecifiedBits | candidateNetmaskSpecifiedBits;
        uint32 newMergedNetmaskIncompatibleBits = mergedNetmaskIncompatibleBits | ((mergedNetmask & commonNetmaskSpecifiedBits) ^ (candidateNetmask & commonNetmaskSpecifiedBits));

        // skip interface if there's a bit where the netmasks are incompatible
        if (newMergedNetmaskIncompatibleBits != 0)
            continue;

        // determine merged address bits
        uint32 commonAddressSpecifiedBits = mergedAddressSpecifiedBits & candidateAddressSpecifiedBits;
        uint32 newMergedAddress = mergedAddress | (candidateAddress & candidateAddressSpecifiedBits);
        uint32 newMergedAddressSpecifiedBits = mergedAddressSpecifiedBits | candidateAddressSpecifiedBits;
        uint32 newMergedAddressIncompatibleBits = mergedAddressIncompatibleBits | ((mergedAddress & commonAddressSpecifiedBits) ^ (candidateAddress & commonAddressSpecifiedBits));

        // skip interface if there's a bit where the netmask is 1 and the addresses are incompatible
        if ((newMergedNetmask & newMergedNetmaskSpecifiedBits & newMergedAddressIncompatibleBits) != 0)
            continue;

        // store merged address bits
        mergedAddress = newMergedAddress;
        mergedAddressSpecifiedBits = newMergedAddressSpecifiedBits;
        mergedAddressIncompatibleBits = newMergedAddressIncompatibleBits;

        // store merged netmask bits
        mergedNetmask = newMergedNetmask;
        mergedNetmaskSpecifiedBits = newMergedNetmaskSpecifiedBits;
        mergedNetmaskIncompatibleBits = newMergedNetmaskIncompatibleBits;

        // add interface to the list of compatible interfaces
        compatibleInterfaces.push_back(candidateInterface);
        EV_DEBUG << "Merged address specification: " << IPv4Address(mergedAddress) << " / " << IPv4Address(mergedAddressSpecifiedBits) << " / " << IPv4Address(mergedAddressIncompatibleBits) << endl;
        EV_DEBUG << "Merged netmask specification: " << IPv4Address(mergedNetmask) << " / " << IPv4Address(mergedNetmaskSpecifiedBits) << " / " << IPv4Address(mergedNetmaskIncompatibleBits) << endl;
    }
    EV_DEBUG << "Found " << compatibleInterfaces.size() << " compatible interfaces" << endl;
}

void IPv4NetworkConfigurator::assignAddresses(IPv4Topology& topology)
{
    int bitSize = sizeof(uint32) * 8;
    std::vector<uint32> assignedNetworkAddresses;
    std::vector<uint32> assignedNetworkNetmasks;
    std::vector<uint32> assignedInterfaceAddresses;
    std::map<uint32, InterfaceEntry *> assignedAddressToInterfaceEntryMap;

    // iterate through all links and process them separately one by one
    for (int linkIndex = 0; linkIndex < (int)topology.linkInfos.size(); linkIndex++)
    {
        LinkInfo *selectedLink = topology.linkInfos[linkIndex];

        std::vector<InterfaceInfo *> unconfiguredInterfaces;
        for (std::vector<InterfaceInfo *>::iterator it = selectedLink->interfaceInfos.begin(); it != selectedLink->interfaceInfos.end(); ++it)
            if ((*it)->interfaceEntry->ipv4Data())
                unconfiguredInterfaces.push_back(*it);
        // repeat until all interfaces of the selected link become configured
        // and assign addresses to groups of interfaces having compatible address and netmask specifications
        while (unconfiguredInterfaces.size() != 0)
        {
            // STEP 1.
            uint32 mergedAddress;                 // compatible bits of the merged address (both 0 and 1 are address bits)
            uint32 mergedAddressSpecifiedBits;    // mask for the valid compatible bits of the merged address (0 means unspecified, 1 means specified)
            uint32 mergedAddressIncompatibleBits; // incompatible bits of the merged address (0 means compatible, 1 means incompatible)
            uint32 mergedNetmask;                 // compatible bits of the merged netmask (both 0 and 1 are netmask bits)
            uint32 mergedNetmaskSpecifiedBits;    // mask for the compatible bits of the merged netmask (0 means unspecified, 1 means specified)
            uint32 mergedNetmaskIncompatibleBits; // incompatible bits of the merged netmask (0 means compatible, 1 means incompatible)
            std::vector<InterfaceInfo *> compatibleInterfaces; // the list of compatible interfaces
            collectCompatibleInterfaces(unconfiguredInterfaces, compatibleInterfaces, mergedAddress, mergedAddressSpecifiedBits, mergedAddressIncompatibleBits, mergedNetmask, mergedNetmaskSpecifiedBits, mergedNetmaskIncompatibleBits);

            // STEP 2.
            // determine the valid range of netmask length by searching from left to right the last 1 and the first 0 bits
            // also consider the incompatible bits of the address to limit the range of valid netmasks accordingly
            int minimumNetmaskLength = bitSize - getLeastSignificantBitIndex(mergedNetmask & mergedNetmaskSpecifiedBits, 1, bitSize); // 0 means 0.0.0.0, bitSize means 255.255.255.255
            int maximumNetmaskLength = bitSize - 1 - getMostSignificantBitIndex(~mergedNetmask & mergedNetmaskSpecifiedBits, 1, -1); // 0 means 0.0.0.0, bitSize means 255.255.255.255
            maximumNetmaskLength = std::min(maximumNetmaskLength, bitSize - 1 - getMostSignificantBitIndex(mergedAddressIncompatibleBits, 1, -1));

            // make sure there are enough bits to configure a unique address for all interface
            // the +2 means that all-0 and all-1 addresses are ruled out
            int compatibleInterfaceCount = compatibleInterfaces.size() + 2;
            int interfaceAddressBitCount = getRepresentationBitCount(compatibleInterfaceCount);
            maximumNetmaskLength = std::min(maximumNetmaskLength, bitSize - interfaceAddressBitCount);
            EV_DEBUG << "Netmask valid length range: " << minimumNetmaskLength << " - " << maximumNetmaskLength << endl;

            // STEP 3.
            // determine network address and network netmask by iterating through valid netmasks from longest to shortest
            int netmaskLength = -1;
            uint32 networkAddress = 0; // network part of the addresses  (e.g. 10.1.1.0)
            uint32 networkNetmask = 0; // netmask for the network (e.g. 255.255.255.0)
            for (netmaskLength = maximumNetmaskLength; netmaskLength >= minimumNetmaskLength; netmaskLength--)
            {
                networkNetmask = (((uint32)1 << netmaskLength) - (uint32)1) << (bitSize - netmaskLength);
                EV_DEBUG << "Trying network netmask: " << IPv4Address(networkNetmask) << " : " << netmaskLength << endl;
                networkAddress = mergedAddress & mergedAddressSpecifiedBits & networkNetmask;
                uint32 networkAddressUnspecifiedBits = ~mergedAddressSpecifiedBits & networkNetmask; // 1 means the network address unspecified
                uint32 networkAddressUnspecifiedPartMaximum = 0;
                for (int i = 0; i < (int)assignedNetworkAddresses.size(); i++)
                {
                    uint32 assignedNetworkAddress = assignedNetworkAddresses[i];
                    uint32 assignedNetworkNetmask = assignedNetworkNetmasks[i];
                    uint32 assignedNetworkAddressMaximum = assignedNetworkAddress | ~assignedNetworkNetmask;
                    EV_DEBUG << "Checking against assigned network address " << IPv4Address(assignedNetworkAddress) << endl;
                    if ((assignedNetworkAddress & ~networkAddressUnspecifiedBits) == (networkAddress & ~networkAddressUnspecifiedBits))
                    {
                        uint32 assignedAddressUnspecifiedPart = getPackedBits(assignedNetworkAddressMaximum, networkAddressUnspecifiedBits);
                        if (assignedAddressUnspecifiedPart > networkAddressUnspecifiedPartMaximum)
                            networkAddressUnspecifiedPartMaximum = assignedAddressUnspecifiedPart;
                    }
                }
                uint32 networkAddressUnspecifiedPartLimit = getPackedBits(~(uint32)0, networkAddressUnspecifiedBits) + (uint32)1;
                EV_DEBUG << "Counting from: " << networkAddressUnspecifiedPartMaximum + (uint32)1 << " to: " << networkAddressUnspecifiedPartLimit << endl;

                // we start with +1 so that the network address will be more likely different
                for (uint32 networkAddressUnspecifiedPart = networkAddressUnspecifiedPartMaximum; networkAddressUnspecifiedPart <= networkAddressUnspecifiedPartLimit; networkAddressUnspecifiedPart++)
                {
                    networkAddress = setPackedBits(networkAddress, networkAddressUnspecifiedBits, networkAddressUnspecifiedPart);
                    EV_DEBUG << "Trying network address: " << IPv4Address(networkAddress) << endl;

                    // count interfaces that have the same address prefix
                    int interfaceCount = 0;
                    for (int i = 0; i < (int)assignedInterfaceAddresses.size(); i++)
                        if ((assignedInterfaceAddresses[i] & networkNetmask) == networkAddress)
                            interfaceCount++;
                    if (assignDisjunctSubnetAddressesParameter && interfaceCount != 0)
                        continue;
                    EV_DEBUG << "Matching interface count: " << interfaceCount << endl;

                    // check if there's enough room for the interface addresses
                    if ((1 << (bitSize - netmaskLength)) >= interfaceCount + compatibleInterfaceCount)
                        goto found;
                }
            }
            found: if (netmaskLength < minimumNetmaskLength || netmaskLength > maximumNetmaskLength)
                throw cRuntimeError("Failed to find address prefix (using %s with specified bits %s) and netmask (length from %d bits to %d bits) for interface %s and %d other interface(s). Please refine your parameters and try again!",
                    IPv4Address(mergedAddress).str().c_str(), IPv4Address(mergedAddressSpecifiedBits).str().c_str(), minimumNetmaskLength, maximumNetmaskLength,
                    compatibleInterfaces[0]->interfaceEntry->getFullPath().c_str(), compatibleInterfaces.size() - 1);
            EV_DEBUG << "Selected netmask length: " << netmaskLength << endl;
            EV_DEBUG << "Selected network address: " << IPv4Address(networkAddress) << endl;
            EV_DEBUG << "Selected network netmask: " << IPv4Address(networkNetmask) << endl;

            // STEP 4.
            // determine the complete IP address for all compatible interfaces
            for (int interfaceIndex = 0; interfaceIndex < (int)compatibleInterfaces.size(); interfaceIndex++)
            {
                InterfaceInfo *compatibleInterface = compatibleInterfaces[interfaceIndex];
                InterfaceEntry *interfaceEntry = compatibleInterface->interfaceEntry;
                uint32 interfaceAddress = compatibleInterface->address & ~networkNetmask;
                uint32 interfaceAddressSpecifiedBits = compatibleInterface->addressSpecifiedBits;
                uint32 interfaceAddressUnspecifiedBits = ~interfaceAddressSpecifiedBits & ~networkNetmask; // 1 means the interface address is unspecified
                uint32 interfaceAddressUnspecifiedPartMaximum = 0;
                for (int i = 0; i < (int)assignedInterfaceAddresses.size(); i++)
                {
                    uint32 otherInterfaceAddress = assignedInterfaceAddresses[i];
                    if ((otherInterfaceAddress & ~interfaceAddressUnspecifiedBits) == ((networkAddress | interfaceAddress) & ~interfaceAddressUnspecifiedBits))
                    {
                        uint32 otherInterfaceAddressUnspecifiedPart = getPackedBits(otherInterfaceAddress, interfaceAddressUnspecifiedBits);
                        if (otherInterfaceAddressUnspecifiedPart > interfaceAddressUnspecifiedPartMaximum)
                            interfaceAddressUnspecifiedPartMaximum = otherInterfaceAddressUnspecifiedPart;
                    }
                }
                interfaceAddressUnspecifiedPartMaximum++;
                interfaceAddress = setPackedBits(interfaceAddress, interfaceAddressUnspecifiedBits, interfaceAddressUnspecifiedPartMaximum);

                // determine the complete address and netmask for interface
                uint32 completeAddress = networkAddress | interfaceAddress;
                uint32 completeNetmask = networkNetmask;

                // check if we could really find a unique IP address
                if (assignedAddressToInterfaceEntryMap.find(completeAddress) != assignedAddressToInterfaceEntryMap.end())
                    throw cRuntimeError("Failed to configure unique address for %s. Please refine your parameters and try again!", interfaceEntry->getFullPath().c_str());
                assignedAddressToInterfaceEntryMap[completeAddress] = compatibleInterface->interfaceEntry;
                assignedInterfaceAddresses.push_back(completeAddress);

                // configure interface with the selected address and netmask
                if (compatibleInterface->configure)
                {
                    IPv4InterfaceData *interfaceData = compatibleInterface->interfaceEntry->ipv4Data();
                    interfaceData->setIPAddress(IPv4Address(completeAddress));
                    interfaceData->setNetmask(IPv4Address(completeNetmask));
                }
                compatibleInterface->address = completeAddress;
                EV_DEBUG << "Selected interface address: " << IPv4Address(completeAddress) << endl;

                // remove configured interface
                unconfiguredInterfaces.erase(find(unconfiguredInterfaces, compatibleInterface));
            }

            // register the network address and netmask as being used
            assignedNetworkAddresses.push_back(networkAddress);
            assignedNetworkNetmasks.push_back(networkNetmask);
        }
    }
}

IPv4NetworkConfigurator::InterfaceInfo *IPv4NetworkConfigurator::createInterfaceInfo(Node *node, LinkInfo* linkInfo, InterfaceEntry *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(node, linkInfo, ie);
    IPv4InterfaceData *ipv4Data = ie->ipv4Data();
    if (ipv4Data)
    {
        IPv4Address address = ipv4Data->getIPAddress();
        IPv4Address netmask = ipv4Data->getNetmask();
        if (!address.isUnspecified())
        {
            interfaceInfo->address = address.getInt();
            interfaceInfo->addressSpecifiedBits = 0xFFFFFFFF;
            interfaceInfo->netmask = netmask.getInt();
            interfaceInfo->netmaskSpecifiedBits = 0xFFFFFFFF;
        }
    }
    node->interfaceInfos.push_back(interfaceInfo);
    return interfaceInfo;
}

IPv4NetworkConfigurator::Matcher::Matcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
        matchers.push_back(new inet::PatternMatcher(tokenizer.nextToken(), true, true, true));
}

IPv4NetworkConfigurator::Matcher::~Matcher()
{
    for (int i = 0; i < (int)matchers.size(); i++)
        delete matchers[i];
}

bool IPv4NetworkConfigurator::Matcher::matches(const char *s)
{
    if (matchesany)
        return true;
    for (int i = 0; i < (int)matchers.size(); i++)
        if (matchers[i]->matches(s))
            return true;
    return false;
}

IPv4NetworkConfigurator::InterfaceMatcher::InterfaceMatcher(const char *pattern)
{
    matchesany = isEmpty(pattern);
    if (matchesany)
        return;
    cStringTokenizer tokenizer(pattern);
    while (tokenizer.hasMoreTokens())
    {
        const char *token = tokenizer.nextToken();
        if (*token == '>')
            towardsMatchers.push_back(new inet::PatternMatcher(token+1, true, true, true));
        else
            nameMatchers.push_back(new inet::PatternMatcher(token, true, true, true));
    }
}

IPv4NetworkConfigurator::InterfaceMatcher::~InterfaceMatcher()
{
    for (int i = 0; i < (int)nameMatchers.size(); i++)
        delete nameMatchers[i];
    for (int i = 0; i < (int)towardsMatchers.size(); i++)
        delete towardsMatchers[i];
}

bool IPv4NetworkConfigurator::InterfaceMatcher::matches(InterfaceInfo *interfaceInfo)
{
    if (matchesany)
        return true;

    const char *interfaceName = interfaceInfo->interfaceEntry->getName();
    for (int i = 0; i < (int)nameMatchers.size(); i++)
        if (nameMatchers[i]->matches(interfaceName))
            return true;

    LinkInfo *linkInfo = interfaceInfo->linkInfo;
    cModule *ownerModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
    for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++)
    {
        InterfaceInfo *candidateInfo = linkInfo->interfaceInfos[i];
        cModule *candidateModule = candidateInfo->interfaceEntry->getInterfaceTable()->getHostModule();
        if (candidateModule == ownerModule)
            continue;
        std::string candidateFullPath = candidateModule->getFullPath();
        std::string candidateShortenedFullPath = candidateFullPath.substr(candidateFullPath.find('.')+1);
        for (int j = 0; j < (int)towardsMatchers.size(); j++)
            if (towardsMatchers[j]->matches(candidateShortenedFullPath.c_str()) ||
                    towardsMatchers[j]->matches(candidateFullPath.c_str()))
                return true;
    }
    return false;
}

inline bool strToBool(const char *str, bool defaultValue)
{
    if (!str || !str[0])
        return defaultValue;
    if (strcmp(str,"true")==0)
        return true;
    if (strcmp(str,"false")==0)
        return false;
    throw cRuntimeError("Invalid boolean XML attribute:'%s'", str);
}

void IPv4NetworkConfigurator::readAddressConfiguration(cXMLElement *root, IPv4Topology& topology)
{
    std::set<InterfaceInfo *> interfacesSeen;
    cXMLElementList interfaceElements = root->getChildrenByTagName("interface");

    for (int i = 0; i < (int)interfaceElements.size(); i++)
    {
        cXMLElement *interfaceElement = interfaceElements[i];
        const char *hostAttr = interfaceElement->getAttribute("hosts");  // "host* router[0..3]"
        const char *interfaceAttr = interfaceElement->getAttribute("names"); // i.e. interface names, like "eth* ppp0"

        // TODO: "switch" egyebkent sztem nem muxik most, de kellene!
        const char *towardsAttr = interfaceElement->getAttribute("towards"); // neighbor host names, like "ap switch"
        const char *amongAttr = interfaceElement->getAttribute("among"); // neighbor host names, like "host[*] router1"
        const char *addressAttr = interfaceElement->getAttribute("address"); // "10.0.x.x"
        const char *netmaskAttr = interfaceElement->getAttribute("netmask"); // "255.255.x.x"
        const char *mtuAttr = interfaceElement->getAttribute("mtu"); // integer
        const char *metricAttr = interfaceElement->getAttribute("metric"); // integer

        if (amongAttr && *amongAttr)       // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
        {
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", interfaceElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try
        {
            // parse host/interface/towards expressions
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);
            Matcher towardsMatcher(towardsAttr);

            // parse address/netmask constraints
            bool haveAddressConstraint = isNotEmpty(addressAttr);
            bool haveNetmaskConstraint = isNotEmpty(netmaskAttr);

            uint32_t address, addressSpecifiedBits, netmask, netmaskSpecifiedBits;
            if (haveAddressConstraint)
                parseAddressAndSpecifiedBits(addressAttr, address, addressSpecifiedBits);
            if (haveNetmaskConstraint)
                parseAddressAndSpecifiedBits(netmaskAttr, netmask, netmaskSpecifiedBits);

            // configure address/netmask constraints on matching interfaces
            for (int i = 0; i < (int)topology.linkInfos.size(); i++)
            {
                LinkInfo *linkInfo = topology.linkInfos[i];
                for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
                {
                    InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[j];
                    if (interfacesSeen.count(interfaceInfo) == 0)
                    {
                        cModule *hostModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
                        std::string hostFullPath = hostModule->getFullPath();
                        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                        // Note: "hosts", "interfaces" and "towards" must ALL match on the interface for the rule to apply
                        if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                            (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceInfo->interfaceEntry->getFullName())) &&
                            (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(linkInfo, &towardsMatcher, hostModule)))
                        {
                            // unicast address constraints
                            interfaceInfo->configure = haveAddressConstraint;
                            if (interfaceInfo->configure)
                            {
                                interfaceInfo->address = address;
                                interfaceInfo->addressSpecifiedBits = addressSpecifiedBits;
                                if (haveNetmaskConstraint)
                                {
                                    interfaceInfo->netmask = netmask;
                                    interfaceInfo->netmaskSpecifiedBits = netmaskSpecifiedBits;
                                }
                            }

                            // mtu
                            if (isNotEmpty(mtuAttr))
                                interfaceInfo->interfaceEntry->setMtu(atoi(mtuAttr));

                            // metric
                            if (isNotEmpty(metricAttr))
                            {
                                ASSERT(interfaceInfo->interfaceEntry->ipv4Data());
                                interfaceInfo->interfaceEntry->ipv4Data()->setMetric(atoi(metricAttr));
                            }

                            interfacesSeen.insert(interfaceInfo);
                            EV_DEBUG << hostModule->getFullPath() << ":" << interfaceInfo->interfaceEntry->getFullName() << endl;
                        }
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Error in XML <interface> element at %s: %s", interfaceElement->getSourceLocation(), e.what());
        }
    }
}

void IPv4NetworkConfigurator::parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits)
{
    // change "10.0.x.x" to "10.0.0.0" (for address) and "255.255.0.0" (for specifiedBits)
    std::string address;
    std::string specifiedBits;
    cStringTokenizer tokenizer(addressAttr, ".");
    while (tokenizer.hasMoreTokens())
    {
        std::string token = tokenizer.nextToken();
        address += (token == "x") ? "0." : (token+".");
        specifiedBits += (token == "x") ? "0." : "255.";
    }
    address = address.substr(0, address.size()-1);
    specifiedBits = specifiedBits.substr(0, specifiedBits.size()-1);

    if (!IPv4Address::isWellFormed(address.c_str()) || !IPv4Address::isWellFormed(specifiedBits.c_str()))
        throw cRuntimeError("Malformed IPv4 address or netmask constraint '%s'", addressAttr);

    outAddress = IPv4Address(address.c_str()).getInt();
    outAddressSpecifiedBits = IPv4Address(specifiedBits.c_str()).getInt();
}

bool IPv4NetworkConfigurator::linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule)
{
    for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++)
    {
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[i];
        cModule *hostModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
        if (hostModule == exceptModule)
            continue;
        std::string hostFullPath = hostModule->getFullPath();
        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
        if (hostMatcher->matches(hostShortenedFullPath.c_str()) || hostMatcher->matches(hostFullPath.c_str()))
            return true;
    }
    return false;
}

void IPv4NetworkConfigurator::dumpTopology(IPv4Topology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        EV_INFO << "Node " << node->module->getFullPath() << endl;
        for (int j = 0; j < node->getNumOutLinks(); j++)
        {
            Topology::LinkOut *linkOut = node->getLinkOut(j);
            ASSERT(linkOut->getLocalNode() == node);
            Node *remoteNode = (Node *)linkOut->getRemoteNode();
            EV_INFO << "     -> " << remoteNode->module->getFullPath() << " " << linkOut->getWeight() << endl;
        }
        for (int j = 0; j < node->getNumInLinks(); j++)
        {
            Topology::LinkIn *linkIn = node->getLinkIn(j);
            ASSERT(linkIn->getLocalNode() == node);
            Node *remoteNode = (Node *)linkIn->getRemoteNode();
            EV_INFO << "     <- " << remoteNode->module->getFullPath() << " " << linkIn->getWeight() << endl;
        }
    }
}

void IPv4NetworkConfigurator::dumpAddresses(IPv4Topology& topology)
{
    for (int i = 0; i < (int)topology.linkInfos.size(); i++)
    {
        EV_INFO << "Link " << i << endl;
        LinkInfo* linkInfo = topology.linkInfos[i];
        for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
        {
            InterfaceEntry *interfaceEntry = linkInfo->interfaceInfos[j]->interfaceEntry;
            cModule *host = dynamic_cast<cModule *>(interfaceEntry->getInterfaceTable())->getParentModule();
            EV_INFO << "    " << host->getFullName() << " / " << interfaceEntry->info() << endl;
        }
    }
}

void IPv4NetworkConfigurator::dumpRoutes(IPv4Topology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable)
        {
            EV_INFO << "Node " << node->module->getFullPath() << endl;
            node->routingTable->printRoutingTable();
            if (node->routingTable->getNumMulticastRoutes() > 0)
                node->routingTable->printMulticastRoutingTable();
        }
    }
}

void IPv4NetworkConfigurator::dumpConfig(IPv4Topology& topology)
{
    FILE *f;
    f = fopen(par("dumpConfig").stringValue(), "w");
    if (!f)
        throw cRuntimeError("Cannot write configurator output file");
    fprintf(f, "<config>\n");

    // interfaces
    for (int i = 0; i < (int)topology.linkInfos.size(); i++)
    {
        LinkInfo* linkInfo = topology.linkInfos[i];
        for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
        {
            InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[j];
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            IPv4InterfaceData *interfaceData = interfaceEntry->ipv4Data();
            std::stringstream stream;
            stream << "   <interface hosts=\"" << interfaceInfo->node->module->getFullPath() << "\" names=\"" << interfaceEntry->getName() <<
                      "\" address=\"" << interfaceData->getIPAddress() << "\" netmask=\"" << interfaceData->getNetmask() <<
                      "\" metric=\"" << interfaceData->getMetric() <<
                      "\"/>" << endl;
            fprintf(f, "%s", stream.str().c_str());
        }
    }

    // multicast groups
    for (int i = 0; i < (int)topology.linkInfos.size(); i++)
    {
        LinkInfo* linkInfo = topology.linkInfos[i];
        for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
        {
            InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[j];
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            IPv4InterfaceData *interfaceData = interfaceEntry->ipv4Data();
            const std::vector<IPv4Address>& multicastAddresses = interfaceData->getJoinedMulticastGroups();

            if (multicastAddresses.size() > 0)
            {
                std::stringstream stream;
                stream << "   <multicast-group hosts=\"" << interfaceInfo->node->module->getFullPath() << "\" interfaces=\"" << interfaceEntry->getName() << "\" address=\"";
                for (int k = 0 ; k < (int)multicastAddresses.size(); k++)
                {
                    IPv4Address address = multicastAddresses[k];
                    if (k) stream << " ";
                    stream << address.str();
                }
                stream << "\"/>" << endl;
                fprintf(f, "%s", stream.str().c_str());
            }
        }
    }

    // wireless links
    for (int i = 0; i < (int)topology.linkInfos.size(); i++)
    {
        LinkInfo* linkInfo = topology.linkInfos[i];
        if (linkInfo->isWireless)
        {
            std::stringstream stream;
            stream << "   <wireless interfaces=\"";
            for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
            {
                if (j) stream << " ";
                InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[j];
                stream << interfaceInfo->node->module->getFullPath() << "%" << interfaceInfo->interfaceEntry->getName();
            }
            stream << "\"/>" << endl;
            fprintf(f, "%s", stream.str().c_str());
        }
    }


    // routes
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        IRoutingTable *routingTable = node->routingTable;
        if (routingTable)
        {
            for (int j = 0; j < routingTable->getNumRoutes(); j++)
            {
                IPv4Route *route = routingTable->getRoute(j);
                std::stringstream stream;
                IPv4Address netmask = route->getNetmask();
                IPv4Address gateway = route->getGateway();
                stream << "   <route hosts=\"" << node->module->getFullPath();
                stream << "\" destination=\""; if (route->getDestination().isUnspecified()) stream << "*"; else stream << route->getDestination();
                stream << "\" netmask=\""; if (route->getNetmask().isUnspecified()) stream << "*"; else stream << route->getNetmask();
                stream << "\" gateway=\""; if (route->getGateway().isUnspecified()) stream << "*"; else stream << route->getGateway();
                stream << "\" interface=\"" << route->getInterfaceName() << "\" metric=\"" << route->getMetric() << "\"/>" << endl;
                fprintf(f, "%s", stream.str().c_str());
            }
        }
    }

    // multicast routes
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        Node *node = (Node *)topology.getNode(i);
        IRoutingTable *routingTable = node->routingTable;
        if (routingTable)
        {
            for (int j = 0; j < routingTable->getNumMulticastRoutes(); j++)
            {
                IPv4MulticastRoute *route = routingTable->getMulticastRoute(j);
                std::stringstream stream;
                stream << "   <multicast-route hosts=\"" << node->module->getFullPath();
                stream << "\" source=\""; if (route->getOrigin().isUnspecified()) stream << "*"; else stream << route->getOrigin();
                stream << "\" netmask=\""; if (route->getOriginNetmask().isUnspecified()) stream << "*"; else stream << route->getOriginNetmask();
                stream << "\" groups=\""; if (route->getMulticastGroup().isUnspecified()) stream << "*"; else stream << route->getMulticastGroup();
                if (route->getParent()) stream << "\" parent=\"" << route->getParent()->getName();
                stream << "\" children=\"";
                for (int k = 0; k < (int)route->getChildren().size(); k++)
                {
                    if (k) stream << " ";
                    stream << route->getChildren()[k]->getInterface()->getName();
                }
                stream << "\" metric=\"" << route->getMetric() << "\"/>" << endl;
                fprintf(f, "%s", stream.str().c_str());
            }
        }
    }

    fprintf(f, "</config>");
    fflush(f);
    fclose(f);
}

void IPv4NetworkConfigurator::addMulticastGroups(cXMLElement *root, IPv4Topology& topology)
{
    cXMLElementList multicastGroupElements = root->getChildrenByTagName("multicast-group");
    for (int i = 0; i < (int)multicastGroupElements.size(); i++)
    {
        cXMLElement *multicastGroupElement = multicastGroupElements[i];
        const char *hostAttr = multicastGroupElement->getAttribute("hosts");
        const char *interfaceAttr = multicastGroupElement->getAttribute("interfaces");
        const char *addressAttr = multicastGroupElement->getAttribute("address");
        const char *towardsAttr = multicastGroupElement->getAttribute("towards"); // neighbor host names, like "ap switch"
        const char *amongAttr = multicastGroupElement->getAttribute("among");

        if (amongAttr && *amongAttr)       // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
        {
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", multicastGroupElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try
        {
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);
            Matcher towardsMatcher(towardsAttr);

            // parse group addresses
            std::vector<IPv4Address> multicastGroups;
            cStringTokenizer tokenizer(addressAttr);
            while (tokenizer.hasMoreTokens())
            {
                IPv4Address addr = IPv4Address(tokenizer.nextToken());
                if (!addr.isMulticast())
                    throw cRuntimeError("Non-multicast address %s found in the multicast-group element", addr.str().c_str());
                multicastGroups.push_back(addr);
            }

            for (int j = 0; j < (int)topology.linkInfos.size(); j++)
            {
                LinkInfo *linkInfo = topology.linkInfos[j];
                for (int k = 0; k < (int)linkInfo->interfaceInfos.size(); k++)
                {
                    InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[k];
                    cModule *hostModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
                    std::string hostFullPath = hostModule->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                    if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                        (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceInfo->interfaceEntry->getFullName())) &&
                        (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(linkInfo, &towardsMatcher, hostModule)))
                    {
                        for (int k = 0; k < (int)multicastGroups.size(); k++)
                            interfaceInfo->interfaceEntry->ipv4Data()->joinMulticastGroup(multicastGroups[k]);
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Error in XML <multicast-group> element at %s: %s", multicastGroupElement->getSourceLocation(), e.what());
        }
    }
}

const char *IPv4NetworkConfigurator::getMandatoryAttribute(cXMLElement *element, const char *attr)
{
    const char *value = element->getAttribute(attr);
    if (isEmpty(value))
        throw cRuntimeError("<%s> element is missing mandatory attribute \"%s\" at %s", element->getTagName(), attr, element->getSourceLocation());
    return value;
}

void IPv4NetworkConfigurator::addManualRoutes(cXMLElement *root, IPv4Topology& topology)
{
    cXMLElementList routeElements = root->getChildrenByTagName("route");
    for (int i = 0; i < (int)routeElements.size(); i++)
    {
        cXMLElement *routeElement = routeElements[i];
        const char *hostAttr = getMandatoryAttribute(routeElement, "hosts");
        const char *destinationAttr = getMandatoryAttribute(routeElement, "destination"); // destination address  (IPvXAddressResolver syntax)
        const char *netmaskAttr = routeElement->getAttribute("netmask"); // default: 255.255.255.255; alternative notation: "/23"
        const char *gatewayAttr = routeElement->getAttribute("gateway"); // next hop address (IPvXAddressResolver syntax)
        const char *interfaceAttr = routeElement->getAttribute("interface"); // output interface name
        const char *metricAttr = routeElement->getAttribute("metric");

        try
        {
            // parse and check the attributes
            IPv4Address destination;
            if (!isEmpty(destinationAttr) && strcmp(destinationAttr, "*"))
                destination = IPvXAddressResolver().resolve(destinationAttr, IPvXAddressResolver::ADDR_IPv4).get4();
            IPv4Address netmask;
            if (!isEmpty(netmaskAttr) && strcmp(netmaskAttr, "*"))
            {
                if (netmaskAttr[0] == '/')
                    netmask = IPv4Address::makeNetmask(atoi(netmaskAttr+1));
                else
                    netmask = IPv4Address(netmaskAttr);
            }
            if (!netmask.isValidNetmask())
                throw cRuntimeError("Wrong netmask %s", netmask.str().c_str());
            if (isEmpty(interfaceAttr) && isEmpty(gatewayAttr))
                throw cRuntimeError("Incomplete route: either gateway or interface (or both) must be specified");

            // find matching host(s), and add the route
            Matcher atMatcher(hostAttr);
            for (int i = 0; i < topology.getNumNodes(); i++)
            {
                // extract source
                Node *node = (Node *)topology.getNode(i);
                if (node->routingTable)
                {
                    std::string hostFullPath = node->module->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                    if (atMatcher.matches(hostShortenedFullPath.c_str()) || atMatcher.matches(hostFullPath.c_str()))
                    {
                        // determine the gateway (its address towards this node!) and the output interface for the route (must be done per node)
                        InterfaceEntry *ie;
                        IPv4Address gateway;
                        resolveInterfaceAndGateway(node, interfaceAttr, gatewayAttr, ie, gateway, topology);

                        // create and add route
                        IPv4Route *route = new IPv4Route();
                        route->setDestination(destination);
                        route->setNetmask(netmask);
                        route->setGateway(gateway); // may be unspecified
                        route->setInterface(ie);
                        if (isNotEmpty(metricAttr))
                            route->setMetric(atoi(metricAttr));
                        node->routingTable->addRoute(route);
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Error in XML <route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void IPv4NetworkConfigurator::addManualMulticastRoutes(cXMLElement *root, IPv4Topology& topology)
{
    cXMLElementList routeElements = root->getChildrenByTagName("multicast-route");
    for (unsigned int i = 0; i < routeElements.size(); i++)
    {
        cXMLElement *routeElement = routeElements[i];
        const char *hostAttr = routeElement->getAttribute("hosts");
        const char *sourceAttr = routeElement->getAttribute("source"); // source address  (IPvXAddressResolver syntax)
        const char *netmaskAttr = routeElement->getAttribute("netmask"); // default: 255.255.255.255; alternative notation: "/23"
        const char *groupsAttr = routeElement->getAttribute("groups"); // addresses of the multicast groups, default: 0.0.0.0, matching all groups
        const char *parentAttr = routeElement->getAttribute("parent"); // name of expected input interface
        const char *childrenAttr = routeElement->getAttribute("children"); // names of output interfaces
        const char *metricAttr = routeElement->getAttribute("metric");

        try
        {
            // parse and check the attributes
            IPv4Address source;
            if (!isEmpty(sourceAttr) && strcmp(sourceAttr, "*"))
                source = IPvXAddressResolver().resolve(sourceAttr, IPvXAddressResolver::ADDR_IPv4).get4();
            IPv4Address netmask;
            if (!isEmpty(netmaskAttr) && strcmp(netmaskAttr, "*"))
            {
                if (netmaskAttr[0] == '/')
                    netmask = IPv4Address::makeNetmask(atoi(netmaskAttr+1));
                else
                    netmask = IPv4Address(netmaskAttr);
            }

            if (!netmask.isValidNetmask())
                throw cRuntimeError("Wrong netmask %s", netmask.str().c_str());

            std::vector<IPv4Address> groups;
            if (isEmpty(groupsAttr))
                groups.push_back(IPv4Address::UNSPECIFIED_ADDRESS);
            else
            {
                cStringTokenizer tokenizer(groupsAttr);
                while (tokenizer.hasMoreTokens())
                {
                    IPv4Address group = IPv4Address(tokenizer.nextToken());
                    if (!group.isMulticast())
                        throw cRuntimeError("Address '%s' in groups attribute is not multicast.", group.str().c_str());
                    groups.push_back(group);
                }
            }

            // find matching host(s), and add the route
            Matcher atMatcher(hostAttr);
            InterfaceMatcher childrenMatcher(childrenAttr);
            for (int i = 0; i < topology.getNumNodes(); i++)
            {
                // extract source
                Node *node = (Node *)topology.getNode(i);
                if (node->routingTable && node->routingTable->isMulticastForwardingEnabled())
                {
                    std::string hostFullPath = node->module->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                    if (atMatcher.matches(hostShortenedFullPath.c_str()) || atMatcher.matches(hostFullPath.c_str()))
                    {
                        InterfaceEntry *parent = NULL;
                        if (!isEmpty(parentAttr))
                        {
                            parent = node->interfaceTable->getInterfaceByName(parentAttr);
                            if (!parent)
                                throw cRuntimeError("Parent interface '%s' not found.", parentAttr);
                            if (!parent->isMulticast())
                                throw cRuntimeError("Parent interface '%s' is not multicast.", parentAttr);
                        }

                        std::vector<InterfaceEntry*> children;
                        for (int j = 0; j < (int)node->interfaceInfos.size(); ++j)
                        {
                            InterfaceInfo *interfaceInfo = node->interfaceInfos[j];
                            InterfaceEntry *ie = interfaceInfo->interfaceEntry;
                            if (ie != parent && ie->isMulticast() && childrenMatcher.matches(interfaceInfo))
                                children.push_back(ie);
                        }

                        for (int j = 0; j < (int)groups.size(); ++j)
                        {
                            // create and add route
                            IPv4MulticastRoute *route = new IPv4MulticastRoute();
                            route->setOrigin(source);
                            route->setOriginNetmask(netmask);
                            route->setMulticastGroup(groups[j]);
                            route->setParent(parent);
                            if (isNotEmpty(metricAttr))
                                route->setMetric(atoi(metricAttr));
                            for (int k = 0; k < (int)children.size(); ++k)
                                route->addChild(children[k], false/*TODO:isLeaf*/);
                            node->routingTable->addMulticastRoute(route);

                        }
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            throw cRuntimeError("Error in XML <multicast-route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void IPv4NetworkConfigurator::resolveInterfaceAndGateway(Node *node, const char *interfaceAttr, const char *gatewayAttr,
        InterfaceEntry *&outIE, IPv4Address& outGateway, IPv4Topology& topology)
{
    // resolve interface name
    if (isEmpty(interfaceAttr))
    {
        outIE = NULL;
    }
    else
    {
        outIE = node->interfaceTable->getInterfaceByName(interfaceAttr);
        if (!outIE)
            throw cRuntimeError("Host/router %s has no interface named \"%s\"",
                    node->module->getFullPath().c_str(), interfaceAttr);
    }

    // if gateway is not specified, we are done
    if (isEmpty(gatewayAttr) || !strcmp(gatewayAttr, "*"))
    {
        outGateway = IPv4Address();
        return; // outInterface also already done -- we're done
    }

    ASSERT(isNotEmpty(gatewayAttr)); // see "if" above

    // check syntax of gatewayAttr, and obtain an initial value
    outGateway = IPvXAddressResolver().resolve(gatewayAttr, IPvXAddressResolver::ADDR_IPv4).get4();

    IPv4Address gatewayAddressOnCommonLink;

    if (!outIE)
    {
        // interface is not specified explicitly -- we must deduce it from the gateway.
        // It is expected that the gateway is on the same link with the configured node,
        // and then we pick the interface which connects to that link.

        // loop through all links, and find the one that contains both the
        // configured node and the gateway
        for (int i = 0; i < (int)topology.linkInfos.size(); i++)
        {
            LinkInfo *linkInfo = topology.linkInfos[i];
            InterfaceInfo *gatewayInterfaceOnLink = findInterfaceOnLinkByNodeAddress(linkInfo, outGateway);
            if (gatewayInterfaceOnLink)
            {
                InterfaceInfo *nodeInterfaceOnLink = findInterfaceOnLinkByNode(linkInfo, node->module);
                if (nodeInterfaceOnLink)
                {
                    outIE = nodeInterfaceOnLink->interfaceEntry;
                    gatewayAddressOnCommonLink = gatewayInterfaceOnLink->interfaceEntry->ipv4Data()->getIPAddress(); // we may need it later
                    break;
                }
            }
        }
        if (!outIE)
            throw cRuntimeError("Host/router %s has no interface towards \"%s\"",
                    node->module->getFullPath().c_str(), gatewayAttr);
    }

    // Now we have both the interface and the gateway. Still, we may need to modify
    // the gateway address by picking the address of a different interface of the gateway --
    // the address of the interface which is towards the configured node (i.e. on the same link)
    //
    // gatewayAttr may be an IP address, or a module name, or modulename+interfacename
    // in a syntax accepted by IPvXAddressResolver. If the gatewayAttr is a concrete IP address
    // or contains a gateway interface name (IPvXAddressResolver accepts it after a "/"), we're done
    if (IPv4Address::isWellFormed(gatewayAttr) || strchr(gatewayAttr, '/') != NULL)
        return;

    // At this point, gatewayAttr must be a modulename string, so we can freely pick the
    // interface that's towards the configured node
    if (!gatewayAddressOnCommonLink.isUnspecified())
        outGateway = gatewayAddressOnCommonLink;
    else {
        // find the gateway interface that's on the same link as outIE

        // first, find which link outIE is on...
        LinkInfo *linkInfo = findLinkOfInterface(topology, outIE);

        // then find which gateway interface is on that link
        InterfaceInfo *gatewayInterface = findInterfaceOnLinkByNodeAddress(linkInfo, outGateway);
        if (gatewayInterface)
            outGateway = gatewayInterface->interfaceEntry->ipv4Data()->getIPAddress();
    }
}

IPv4NetworkConfigurator::InterfaceInfo *IPv4NetworkConfigurator::findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node)
{
    for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++)
    {
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[i];
        if (interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule() == node)
            return interfaceInfo;
    }
    return NULL;
}

IPv4NetworkConfigurator::InterfaceInfo *IPv4NetworkConfigurator::findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, IPv4Address address)
{
    for (int i = 0; i < (int)linkInfo->interfaceInfos.size(); i++)
    {
        // if the interface has this address, found
        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[i];
        if (interfaceInfo->interfaceEntry->ipv4Data()->getIPAddress() == address)
            return interfaceInfo;

        // if some other interface of the same node has the address, we accept that too
        IInterfaceTable *interfaceTable = interfaceInfo->interfaceEntry->getInterfaceTable();
        for (int j = 0; j < interfaceTable->getNumInterfaces(); j++)
            if (interfaceTable->getInterface(j)->ipv4Data()->getIPAddress() == address)
                return interfaceInfo;
    }
    return NULL;
}

IPv4NetworkConfigurator::LinkInfo *IPv4NetworkConfigurator::findLinkOfInterface(IPv4Topology& topology, InterfaceEntry *ie)
{
    for (int i = 0; i < (int)topology.linkInfos.size(); i++)
    {
        LinkInfo *linkInfo = topology.linkInfos[i];
        for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++)
            if (linkInfo->interfaceInfos[j]->interfaceEntry == ie)
                return linkInfo;
    }
    return NULL;
}

bool IPv4NetworkConfigurator::containsRoute(const std::vector<IPv4Route *>& routes, IPv4Route *route)
{
    for (int i = 0; i < (int)routes.size(); i++)
        if (*routes.at(i) == *route)
            return true;
    return false;
}

void IPv4NetworkConfigurator::addStaticRoutes(IPv4Topology& topology)
{
    long optimizeRoutesDuration = 0;
    long addDefaultRoutesDuration = 0;
    long calculateShortestPathsDuration = 0;

    // TODO: it should be configurable (via xml?) which nodes need static routes filled in automatically
    // add static routes for all routing tables
    for (int i = 0; i < topology.getNumNodes(); i++)
    {
        // extract source
        Node *sourceNode = (Node *)topology.getNode(i);
        if (!sourceNode->interfaceTable)
            continue;
        IRoutingTable *sourceRoutingTable = sourceNode->routingTable;
        if (!sourceRoutingTable)
            continue;
        //IInterfaceTable *sourceInterfaceTable = sourceNode->interfaceTable;

        // calculate shortest paths from everywhere to sourceNode
        // we are going to use the paths in reverse direction (assuming all links are bidirectional)
        long begin = clock();
        topology.calculateUnweightedSingleShortestPathsTo(sourceNode);
        calculateShortestPathsDuration += clock() - begin;

        // check if adding the default routes would be ok (this is an optimization)
        if (addDefaultRoutesParameter && sourceNode->interfaceInfos.size() == 1 && sourceNode->interfaceInfos[0]->linkInfo->gatewayInterfaceInfo)
        {
            begin = clock();
            InterfaceInfo *sourceInterfaceInfo = sourceNode->interfaceInfos[0];
            InterfaceEntry *sourceInterfaceEntry = sourceInterfaceInfo->interfaceEntry;
            InterfaceInfo *gatewayInterfaceInfo = sourceInterfaceInfo->linkInfo->gatewayInterfaceInfo;
            InterfaceEntry *gatewayInterfaceEntry = gatewayInterfaceInfo->interfaceEntry;

            // add a network route for the local network using ARP
            IPv4Route *route = new IPv4Route();
            IPv4InterfaceData *ipv4InterfaceData = sourceInterfaceEntry->ipv4Data();
            IPv4Address address = ipv4InterfaceData->getIPAddress();
            IPv4Address netmask = ipv4InterfaceData->getNetmask();
            route->setDestination(IPv4Address(address.getInt() & netmask.getInt()));
            route->setGateway(IPv4Address::UNSPECIFIED_ADDRESS);
            route->setNetmask(netmask);
            route->setInterface(sourceInterfaceEntry);
            route->setSource(IPv4Route::MANUAL);
            sourceRoutingTable->addRoute(route);

            // add a default route towards the only one gateway
            route = new IPv4Route();
            IPv4Address gateway = gatewayInterfaceEntry->ipv4Data()->getIPAddress();
            route->setDestination(IPv4Address::UNSPECIFIED_ADDRESS);
            route->setNetmask(IPv4Address::UNSPECIFIED_ADDRESS);
            route->setGateway(gateway);
            route->setInterface(sourceInterfaceEntry);
            route->setSource(IPv4Route::MANUAL);
            sourceRoutingTable->addRoute(route);

            // skip building and optimizing the whole routing table
            EV_DEBUG << "Adding default routes to " << sourceNode->getModule()->getFullPath() << ", node has only one (non-loopback) interface\n";
            addDefaultRoutesDuration += clock() - begin;
        }
        else
        {
            std::vector<IPv4Route *> sourceRoutes;

            // add a route to all destinations in the network
            for (int j = 0; j < topology.getNumNodes(); j++)
            {
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
                while (node != sourceNode)
                {
                    link = (Link *)node->getPath(0);
                    if (node->interfaceTable && node != sourceNode && link->sourceInterfaceInfo)
                        nextHopInterfaceEntry = link->sourceInterfaceInfo->interfaceEntry;
                    node = (Node *)node->getPath(0)->getRemoteNode();
                }

                // determine source interface
                InterfaceEntry *sourceInterfaceEntry = link->destinationInterfaceInfo->interfaceEntry;

                // add the same routes for all destination interfaces (IP packets are accepted from any interface at the destination)
                for (int j = 0; j < destinationInterfaceTable->getNumInterfaces(); j++)
                {
                    InterfaceEntry *destinationInterfaceEntry = destinationInterfaceTable->getInterface(j);
                    if (!destinationInterfaceEntry->ipv4Data())
                        continue;
                    IPv4Address destinationAddress = destinationInterfaceEntry->ipv4Data()->getIPAddress();
                    IPv4Address destinationNetmask = destinationInterfaceEntry->ipv4Data()->getNetmask();
                    if (!destinationInterfaceEntry->isLoopback() && !destinationAddress.isUnspecified() && nextHopInterfaceEntry->ipv4Data())
                    {
                        IPv4Route *route = new IPv4Route();
                        IPv4Address gatewayAddress = nextHopInterfaceEntry->ipv4Data()->getIPAddress();
                        if (addSubnetRoutesParameter && destinationNode->interfaceInfos.size() == 1 && destinationNode->interfaceInfos[0]->linkInfo->gatewayInterfaceInfo)
                        {
                            route->setDestination(destinationAddress.doAnd(destinationNetmask));
                            route->setNetmask(destinationNetmask);
                        }
                        else
                        {
                            route->setDestination(destinationAddress);
                            route->setNetmask(IPv4Address::ALLONES_ADDRESS);
                        }
                        route->setInterface(sourceInterfaceEntry);
                        if (gatewayAddress != destinationAddress)
                            route->setGateway(gatewayAddress);
                        route->setSource(IPv4Route::MANUAL);
                        if (containsRoute(sourceRoutes, route))
                            delete route;
                        else {
                            sourceRoutes.push_back(route);
                            EV_DEBUG << "Adding route " << sourceInterfaceEntry->getFullPath() << " -> " << destinationInterfaceEntry->getFullPath() << " as " << route->info() << endl;
                        }
                    }
                }
            }

            // optimize routing table to save memory and increase lookup performance
            if (optimizeRoutesParameter)
            {
                long begin = clock();
                optimizeRoutes(sourceRoutes);
                optimizeRoutesDuration += clock() - begin;
            }

            // copy into routing table
            for (int i = 0; i < (int)sourceRoutes.size(); i++)
                sourceRoutingTable->addRoute(sourceRoutes[i]);
        }
    }

    // print some timing information
    printTimeSpentUsingDuration("calculateShortestPaths", calculateShortestPathsDuration);
    printTimeSpentUsingDuration("addDefaultRoutes", addDefaultRoutesDuration);
    printTimeSpentUsingDuration("optimizeRoutes", optimizeRoutesDuration);
}

/**
 * Returns true if the two routes are the same except their address prefix and netmask.
 * If it returns true we say that the routes have the same color.
 */
bool IPv4NetworkConfigurator::routesHaveSameColor(IPv4Route *route1, IPv4Route *route2)
{
    return route1->getSource() == route2->getSource() && route1->getMetric() == route2->getMetric() &&
           route1->getGateway() == route2->getGateway() && route1->getInterface() == route2->getInterface();
}

/**
 * Returns the index of the first route that has the same color.
 */
int IPv4NetworkConfigurator::findRouteIndexWithSameColor(const std::vector<IPv4Route *>& routes, IPv4Route *route)
{
    for (int i = 0; i < (int)routes.size(); i++)
        if (routesHaveSameColor(routes[i], route))
            return i;
    return -1;
}

/**
 * Returns true if swapping two ADJACENT routes in the routing table does not change the table's meaning.
 */
bool IPv4NetworkConfigurator::routesCanBeSwapped(RouteInfo *routeInfo1, RouteInfo *routeInfo2)
{
    if (routeInfo1->color == routeInfo2->color)
        return true;  // these two routes send the packet in the same direction (same gw/iface), doesn't matter which one we use -> can be swapped
    else {
        // unrelated routes can also be swapped
        uint32 netmask = routeInfo1->netmask & routeInfo2->netmask;
        return (routeInfo1->destination & netmask) != (routeInfo2->destination & netmask);
    }
}

/**
 * Returns true if the routes can be neighbors by repeatedly swapping routes
 * in the routing table without changing their meaning.
 */
bool IPv4NetworkConfigurator::routesCanBeNeighbors(const std::vector<RouteInfo *>& routeInfos, int i, int j)
{
    int begin = std::min(i, j);
    int end = std::max(i, j);
    IPv4NetworkConfigurator::RouteInfo *beginRouteInfo = routeInfos.at(begin);
    for (int index = begin + 1; index < end; index++)
        if (!routesCanBeSwapped(beginRouteInfo, routeInfos.at(index)))
            return false;
    return true;
}

/**
 * Returns true if the original route is interrupted by any of the routes in
 * the routing table between begin and end.
 */
bool IPv4NetworkConfigurator::interruptsOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, RouteInfo *originalRouteInfo)
{
    IPv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination, begin, end);
    return matchingRouteInfo && matchingRouteInfo->color != originalRouteInfo->color;
}

/**
 * Returns true if any of the original routes is interrupted by any of the
 * routes in the routing table between begin and end.
 */
bool IPv4NetworkConfigurator::interruptsAnyOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos)
{
    if (begin < end)
        for (int i = 0; i < (int)originalRouteInfos.size(); i++)
            if (interruptsOriginalRoute(routingTableInfo, begin, end, originalRouteInfos.at(i)))
                return true;
    return false;
}

/**
 * Returns true if any of the original routes attached to the routes in the
 * routing table below index are interrupted by the route at index.
 */
bool IPv4NetworkConfigurator::interruptsSubsequentOriginalRoutes(const RoutingTableInfo& routingTableInfo, int index)
{
    for (int i = index + 1; i < (int)routingTableInfo.routeInfos.size(); i++)
    {
        IPv4NetworkConfigurator::RouteInfo *routeInfo = routingTableInfo.routeInfos.at(i);
        if (interruptsAnyOriginalRoute(routingTableInfo, index, index + 1, routeInfo->originalRouteInfos))
            return true;
    }
    return false;
}

/**
 * Asserts that all original routes are still routed the same way as by the original routing table.
 */
void IPv4NetworkConfigurator::checkOriginalRoutes(const RoutingTableInfo& routingTableInfo, const std::vector<RouteInfo *>& originalRouteInfos)
{
    // assert that all original routes are routed with the same color
    for (int i = 0; i < (int)originalRouteInfos.size(); i++)
    {
        IPv4NetworkConfigurator::RouteInfo *originalRouteInfo = originalRouteInfos.at(i);
        IPv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination);
        if (!(matchingRouteInfo && matchingRouteInfo->color == originalRouteInfo->color))
            ASSERT(false);
    }
}

/**
 * Returns the longest shared address prefix and netmask by iterating through bits from left to right.
 */
void IPv4NetworkConfigurator::findLongestCommonDestinationPrefix(uint32 destination1, uint32 netmask1, uint32 destination2, uint32 netmask2, uint32& destinationOut, uint32& netmaskOut)
{
    netmaskOut = 0;
    destinationOut = 0;
    for (int bitIndex = 31; bitIndex >= 0; bitIndex--)
    {
        uint32 mask = 1 << bitIndex;
        if ((destination1 & mask) == (destination2 & mask) &&
            (netmask1 & mask) != 0 && (netmask2 & mask) != 0)
        {
            netmaskOut |= mask;
            destinationOut |= destination1 & mask;
        }
        else
            break;
    }
}

/**
 * Adds all of the original routes to the matching optimized routes between begin and end.
 */
void IPv4NetworkConfigurator::addOriginalRouteInfos(RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos)
{
    for (int i = 0; i < (int)originalRouteInfos.size(); i++)
    {
        IPv4NetworkConfigurator::RouteInfo *originalRouteInfo = originalRouteInfos.at(i);
        IPv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination, begin, end);
        ASSERT(matchingRouteInfo && matchingRouteInfo->color == originalRouteInfo->color);
        matchingRouteInfo->originalRouteInfos.push_back(originalRouteInfo);
    }
}

/**
 * Try to merge two routes that have the same color and could be neighbours in table
 * without changing the table's meaning. There are two merge opportunities:
 * (1) one route's network contains the other one (then the second can be dropped),
 * and (2) use a shorter common prefix that covers both (this is only possible
 * if this doesn't interfere with existing routes in the table).
 * Returns true if the two routes have been merged, otherwise returns false.
 */
bool IPv4NetworkConfigurator::tryToMergeTwoRoutes(RoutingTableInfo& routingTableInfo, int i, int j, RouteInfo *routeInfoI, RouteInfo *routeInfoJ)
{
    // determine longest shared address prefix and netmask by iterating through bits from left to right
    uint32 netmask;
    uint32 destination;
    findLongestCommonDestinationPrefix(routeInfoI->destination, routeInfoI->netmask, routeInfoJ->destination, routeInfoJ->netmask, destination, netmask);

    // create the merged route
    RouteInfo *mergedRouteInfo = new RouteInfo(routeInfoI->color, destination, netmask);
    routeInfoI->enabled = false;
    routeInfoJ->enabled = false;
    int m = routingTableInfo.addRouteInfo(mergedRouteInfo);
    ASSERT(m > i && m > j);

    // check if all the original routes are still routed the same way by the optimized routing table.
    // check optimization: instead of checking all the original routes, check only those which can go wrong due to the merge.
    // (assuming the previous configuration was correct)
    //  - the original routes on I and J are going to be routed by M after the merge, so check if the routes in between don't interrupt them
    //  - the original routes following M can be accidentally overridden by M (being larger than the sum of I and J), so verify that M does not interrupt them
    // note that the condition is not symmetric because I follows J so it requires fewer checks and we do use that.
    if (!interruptsAnyOriginalRoute(routingTableInfo, j + 1, i, routeInfoJ->originalRouteInfos) && // check that original routes on J are not interrupted between J and I
        !interruptsAnyOriginalRoute(routingTableInfo, i + 1, m, routeInfoJ->originalRouteInfos) && // check that original routes on J are not interrupted between I and M
        !interruptsAnyOriginalRoute(routingTableInfo, i + 1, m, routeInfoI->originalRouteInfos) && // check that original routes on I are not interrupted between I and M
        !interruptsSubsequentOriginalRoutes(routingTableInfo, m)) // check that the original routes after M are not interrupted by M
    {
        // now we know that the merge does not conflict with any route in the routing table.
        // the next thing to do is to maintain the original routes attached to the optimized ones.
        // move original routes from the to be deleted I route to the capturing routes.
        addOriginalRouteInfos(routingTableInfo, i + 1, m + 1, routeInfoI->originalRouteInfos);

        // move original routes from the to be deleted J route to the capturing routes.
        addOriginalRouteInfos(routingTableInfo, j + 1, m + 1, routeInfoJ->originalRouteInfos);

        // move original routes from the routes following the merged one if necessary.
        for (int k = m + 1; k < (int)routingTableInfo.routeInfos.size(); k++)
        {
            IPv4NetworkConfigurator::RouteInfo *followingRouteInfo = routingTableInfo.routeInfos.at(k);
            for (int l = 0; l < (int)followingRouteInfo->originalRouteInfos.size(); l++)
            {
                IPv4NetworkConfigurator::RouteInfo *originalRouteInfo = followingRouteInfo->originalRouteInfos.at(l);
                if (!((originalRouteInfo->destination ^ mergedRouteInfo->destination) & mergedRouteInfo->netmask))
                {
                    followingRouteInfo->originalRouteInfos.erase(followingRouteInfo->originalRouteInfos.begin() + l);
                    ASSERT(mergedRouteInfo->color == originalRouteInfo->color);
                    mergedRouteInfo->originalRouteInfos.push_back(originalRouteInfo);
                    l--;
                }
            }
        }
        routingTableInfo.removeRouteInfo(routeInfoI);
        routingTableInfo.removeRouteInfo(routeInfoJ);
#ifndef NDEBUG
        //checkOriginalRoutes(routingTableInfo, originalRouteInfos);
#endif
        delete routeInfoI;
        delete routeInfoJ;
        return true;
    }
    else
    {
        // merge failed; restore original state
        routeInfoI->enabled = true;
        routeInfoJ->enabled = true;
        routingTableInfo.removeRouteInfo(mergedRouteInfo);
        delete mergedRouteInfo;
        return false;
    }
}

/**
 * Iteratively checks if any two routes can be aggressively merged without changing the meaning of all original routes.
 * The merged route will have the longest shared address prefix and netmask with the two merged routes.
 * This optimization might change the meaning of the routing table in that it will route packets that it did not route before.
 * Nevertheless, any packet routed by the original routing table will still be routed the same way by the optimized routing table.
 * Returns true if two routes has been merged, otherwise returns false.
 */
bool IPv4NetworkConfigurator::tryToMergeAnyTwoRoutes(RoutingTableInfo& routingTableInfo)
{
    for (int i = 0; i < (int)routingTableInfo.routeInfos.size(); i++)
    {
        IPv4NetworkConfigurator::RouteInfo *routeInfoI = routingTableInfo.routeInfos.at(i);

        // iterate backward so that we try to merge routes having longer netmasks first.
        // this results in smaller changes and allows more symmetric optimization.
        for (int j = i - 1; j >= 0; j--)
        {
            IPv4NetworkConfigurator::RouteInfo *routeInfoJ = routingTableInfo.routeInfos.at(j);

            // we can only merge neighbor routes having the same color
            if (routeInfoI->color == routeInfoJ->color && routesCanBeNeighbors(routingTableInfo.routeInfos, i, j))
            {
                // it is worth to actually try to merge them
                if (tryToMergeTwoRoutes(routingTableInfo, i, j, routeInfoI, routeInfoJ))
                    return true;
            }
        }
    }
    return false;
}

void IPv4NetworkConfigurator::optimizeRoutes(std::vector<IPv4Route *>& originalRoutes)
{
    // The basic idea: if two routes "do the same" (same output interface, gateway, etc) and
    // match "similar" addresses, one can try to move them to be neighbors in the table and
    // replace them with a single route which contains their longest common prefix as address
    // prefix -- provided that this operation doesn't affect the meaning of the routing table
    // for "existing" addresses. (We don't care about changing the routing for addresses that
    // we know don't occur in our currently configured network.) We can repeatedly merge routes
    // in this way until it's not possible any more. Of course, the result also depends on
    // which pairs of routes we merge, and in which order.

    // STEP 1.
    // instead of working with IPv4 routes we transform them into the internal representation of the optimizer.
    // routes are classified based on their action (gateway, interface, type, source, metric, etc.) and a color is assigned to them.
    RoutingTableInfo routingTableInfo;
    std::vector<IPv4Route *> colorToRoute;  // a mapping from color to route action (interface, gateway, metric, etc.)
    std::vector<RouteInfo *> originalRouteInfos; // a copy of the original routes in the optimizer's format

    // build colorToRouteColor, originalRouteInfos and initial routeInfos in routingTableInfo
    for (int i = 0; i < (int)originalRoutes.size(); i++)
    {
        IPv4Route *originalRoute = originalRoutes.at(i);
        int color = findRouteIndexWithSameColor(colorToRoute, originalRoute);
        if (color == -1)
        {
            color = colorToRoute.size();
            colorToRoute.push_back(originalRoute);
        }

        // create original route and determine its color
        RouteInfo *originalRouteInfo = new RouteInfo(color, originalRoute->getDestination().getInt(), originalRoute->getNetmask().getInt());
        originalRouteInfos.push_back(originalRouteInfo);

        // create a copy of the original route that can be destructively optimized later
        RouteInfo *optimizedRouteInfo = new RouteInfo(*originalRouteInfo);
        optimizedRouteInfo->originalRouteInfos.push_back(originalRouteInfo);
        routingTableInfo.addRouteInfo(optimizedRouteInfo);
    }

#ifndef NDEBUG
    checkOriginalRoutes(routingTableInfo, originalRouteInfos);
#endif

    // STEP 2.
    // from now on we are only working with the internal data structures called RouteInfo and RoutingTableInfo.
    // the main optimizer loop runs until it cannot merge any two routes.
    while (tryToMergeAnyTwoRoutes(routingTableInfo));

#ifndef NDEBUG
    checkOriginalRoutes(routingTableInfo, originalRouteInfos);
#endif

    // STEP 3.
    // convert the optimized routes to new optimized IPv4 routes based on the saved colors
    std::vector<IPv4Route *> optimizedRoutes;
    for (int i = 0; i < (int)routingTableInfo.routeInfos.size(); i++)
    {
        RouteInfo *routeInfo = routingTableInfo.routeInfos.at(i);
        IPv4Route *routeColor = colorToRoute[routeInfo->color];
        IPv4Route *optimizedRoute = new IPv4Route();
        optimizedRoute->setDestination(IPv4Address(routeInfo->destination));
        optimizedRoute->setNetmask(IPv4Address(routeInfo->netmask));
        optimizedRoute->setInterface(routeColor->getInterface());
        optimizedRoute->setGateway(routeColor->getGateway());
        optimizedRoute->setSource(routeColor->getSource());
        optimizedRoute->setMetric(routeColor->getMetric());
        optimizedRoutes.push_back(optimizedRoute);
        delete routeInfo;
    }

    // delete original routes, we destructively modify them
    for (int i = 0; i < (int)originalRoutes.size(); i++)
        delete originalRoutes.at(i);

    // copy optimized routes to original routes and return
    originalRoutes = optimizedRoutes;
}
