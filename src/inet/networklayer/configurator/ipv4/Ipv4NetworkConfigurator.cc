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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"

namespace inet {

Define_Module(Ipv4NetworkConfigurator);

#define ADDRLEN_BITS    32

Ipv4NetworkConfigurator::InterfaceInfo::InterfaceInfo(Node *node, LinkInfo *linkInfo, InterfaceEntry *interfaceEntry) :
    NetworkConfiguratorBase::InterfaceInfo(node, linkInfo, interfaceEntry),
    address(0),
    addressSpecifiedBits(0),
    netmask(0),
    netmaskSpecifiedBits(0)
{
}

int Ipv4NetworkConfigurator::RoutingTableInfo::addRouteInfo(RouteInfo *routeInfo)
{
    auto it = upper_bound(routeInfos.begin(), routeInfos.end(), routeInfo, routeInfoLessThan);
    int index = it - routeInfos.begin();
    routeInfos.insert(it, routeInfo);
    return index;
}

Ipv4NetworkConfigurator::RouteInfo *Ipv4NetworkConfigurator::RoutingTableInfo::findBestMatchingRouteInfo(const std::vector<RouteInfo *>& routeInfos, const uint32 destination, int begin, int end)
{
    for (int index = begin; index < end; index++) {
        RouteInfo *routeInfo = routeInfos.at(index);
        if (routeInfo->enabled && !((destination ^ routeInfo->destination) & routeInfo->netmask))
            return const_cast<RouteInfo *>(routeInfo);
    }
    return nullptr;
}

void Ipv4NetworkConfigurator::initialize(int stage)
{
    NetworkConfiguratorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        assignAddressesParameter = par("assignAddresses");
        assignUniqueAddresses = par("assignUniqueAddresses");
        assignDisjunctSubnetAddressesParameter = par("assignDisjunctSubnetAddresses");
        addStaticRoutesParameter = par("addStaticRoutes");
        addSubnetRoutesParameter = par("addSubnetRoutes");
        addDefaultRoutesParameter = par("addDefaultRoutes");
        addDirectRoutesParameter = par("addDirectRoutes");
        optimizeRoutesParameter = par("optimizeRoutes");
    }
    else if (stage == INITSTAGE_NETWORK_CONFIGURATION)
        ensureConfigurationComputed(topology);
    else if (stage == INITSTAGE_LAST)
        dumpConfiguration();
}

void Ipv4NetworkConfigurator::computeConfiguration()
{
    EV_INFO << "Computing static network configuration (addresses and routes).\n";
    long initializeStartTime = clock();
    topology.clear();
    // extract topology into the Topology object, then fill in a LinkInfo[] vector
    TIME(extractTopology(topology));
    // read the configuration from XML; it will serve as input for address assignment
    TIME(readInterfaceConfiguration(topology));
    // assign addresses to Ipv4 nodes
    if (assignAddressesParameter)
        TIME(assignAddresses(topology));
    // read and configure multicast groups from the XML configuration
    TIME(readMulticastGroupConfiguration(topology));
    // read and configure manual routes from the XML configuration
    readManualRouteConfiguration(topology);
    // read and configure manual multicast routes from the XML configuration
    readManualMulticastRouteConfiguration(topology);
    // calculate shortest paths, and add corresponding static routes
    if (addStaticRoutesParameter) {
        cXMLElementList autorouteElements = configuration->getChildrenByTagName("autoroute");
        if (autorouteElements.size() == 0) {
            cXMLElement defaultAutorouteElement("autoroute", "", nullptr);
            TIME(addStaticRoutes(topology, &defaultAutorouteElement));
        }
        else {
            for (auto & autorouteElement : autorouteElements)
                TIME(addStaticRoutes(topology, autorouteElement));
        }
    }
    printElapsedTime("computeConfiguration", initializeStartTime);
}

void Ipv4NetworkConfigurator::ensureConfigurationComputed(Topology& topology)
{
    if (topology.getNumNodes() == 0)
        computeConfiguration();
}

void Ipv4NetworkConfigurator::dumpConfiguration()
{
    // print topology to module output
    if (par("dumpTopology"))
        TIME(dumpTopology(topology));
    // print links to module output
    if (par("dumpLinks"))
        TIME(dumpLinks(topology));
    // print unicast and multicast addresses and other interface data to module output
    if (par("dumpAddresses"))
        TIME(dumpAddresses(topology));
    // print routes to module output
    if (par("dumpRoutes"))
        TIME(dumpRoutes(topology));
    // print current configuration to an XML file
    if (!isEmpty(par("dumpConfig")))
        TIME(dumpConfig(topology));
}

void Ipv4NetworkConfigurator::configureAllInterfaces()
{
    ensureConfigurationComputed(topology);
    EV_INFO << "Configuring all network interfaces.\n";
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (auto & elem : node->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(elem);
            if (interfaceInfo->configure)
                configureInterface(interfaceInfo);
        }
    }
}

void Ipv4NetworkConfigurator::configureInterface(InterfaceEntry *interfaceEntry)
{
    ensureConfigurationComputed(topology);
    auto it = topology.interfaceInfos.find(interfaceEntry->getId());
    if (it != topology.interfaceInfos.end()) {
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(it->second);
        if (interfaceInfo->configure)
            configureInterface(interfaceInfo);
    }
}

void Ipv4NetworkConfigurator::configureAllRoutingTables()
{
    ensureConfigurationComputed(topology);
    EV_INFO << "Configuring all routing tables.\n";
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable)
            configureRoutingTable(node);
    }
}

void Ipv4NetworkConfigurator::configureRoutingTable(IIpv4RoutingTable *routingTable)
{
    ensureConfigurationComputed(topology);
    // TODO: avoid linear search
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable == routingTable)
            configureRoutingTable(node);
    }
}

void Ipv4NetworkConfigurator::configureRoutingTable(IIpv4RoutingTable *routingTable, InterfaceEntry *interfaceEntry)
{
    ensureConfigurationComputed(topology);
    // TODO: avoid linear search
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable == routingTable)
            configureRoutingTable(node, interfaceEntry);
    }
}

void Ipv4NetworkConfigurator::configureInterface(InterfaceInfo *interfaceInfo)
{
    EV_DETAIL << "Configuring network interface " << interfaceInfo->getFullPath() << ".\n";
    InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
    Ipv4InterfaceData *interfaceData = interfaceEntry->getProtocolData<Ipv4InterfaceData>();
    if (interfaceInfo->mtu != -1)
        interfaceEntry->setMtu(interfaceInfo->mtu);
    if (interfaceInfo->metric != -1)
        interfaceData->setMetric(interfaceInfo->metric);
    if (assignAddressesParameter) {
        interfaceData->setIPAddress(Ipv4Address(interfaceInfo->address));
        interfaceData->setNetmask(Ipv4Address(interfaceInfo->netmask));
    }
    // TODO: should we leave joined multicast groups first?
    for (auto & multicastGroup : interfaceInfo->multicastGroups)
        interfaceData->joinMulticastGroup(multicastGroup);
}

void Ipv4NetworkConfigurator::configureRoutingTable(Node *node)
{
    EV_DETAIL << "Configuring routing table of " << node->getModule()->getFullPath() << ".\n";
    for (size_t i = 0; i < node->staticRoutes.size(); i++) {
        Ipv4Route *original = node->staticRoutes[i];
        Ipv4Route *clone = new Ipv4Route();
        clone->setMetric(original->getMetric());
        clone->setSourceType(original->getSourceType());
        clone->setSource(original->getSource());
        clone->setDestination(original->getDestination());
        clone->setNetmask(original->getNetmask());
        clone->setGateway(original->getGateway());
        clone->setInterface(original->getInterface());
        node->routingTable->addRoute(clone);
    }
    for (size_t i = 0; i < node->staticMulticastRoutes.size(); i++) {
        Ipv4MulticastRoute *original = node->staticMulticastRoutes[i];
        Ipv4MulticastRoute *clone = new Ipv4MulticastRoute();
        clone->setMetric(original->getMetric());
        clone->setSourceType(original->getSourceType());
        clone->setSource(original->getSource());
        clone->setOrigin(original->getOrigin());
        clone->setOriginNetmask(original->getOriginNetmask());
        clone->setInInterface(original->getInInterface());
        clone->setMulticastGroup(original->getMulticastGroup());
        for (size_t j = 0; j < original->getNumOutInterfaces(); j++)
            clone->addOutInterface(new IMulticastRoute::OutInterface(*original->getOutInterface(j)));
        node->routingTable->addMulticastRoute(clone);
    }
}

void Ipv4NetworkConfigurator::configureRoutingTable(Node *node, InterfaceEntry *interfaceEntry)
{
    EV_DETAIL << "Configuring routing table of " << node->getModule()->getFullPath() << ".\n";
    for (size_t i = 0; i < node->staticRoutes.size(); i++) {
        Ipv4Route *original = node->staticRoutes[i];
        if (original->getInterface() == interfaceEntry) {
            Ipv4Route *clone = new Ipv4Route();
            clone->setMetric(original->getMetric());
            clone->setSourceType(original->getSourceType());
            clone->setSource(original->getSource());
            clone->setDestination(original->getDestination());
            clone->setNetmask(original->getNetmask());
            clone->setGateway(original->getGateway());
            clone->setInterface(original->getInterface());
            node->routingTable->addRoute(clone);
        }
    }
    for (size_t i = 0; i < node->staticMulticastRoutes.size(); i++) {
        Ipv4MulticastRoute *original = node->staticMulticastRoutes[i];
        bool needed = original->getInInterface() && original->getInInterface()->getInterface() == interfaceEntry;
        for (size_t j = 0; !needed && j < original->getNumOutInterfaces(); j++)
            if (original->getOutInterface(j) && original->getOutInterface(j)->getInterface() == interfaceEntry)
                needed = true;

        if (needed) {
            Ipv4MulticastRoute *clone = new Ipv4MulticastRoute();
            clone->setMetric(original->getMetric());
            clone->setSourceType(original->getSourceType());
            clone->setSource(original->getSource());
            clone->setOrigin(original->getOrigin());
            clone->setOriginNetmask(original->getOriginNetmask());
            clone->setInInterface(original->getInInterface());
            clone->setMulticastGroup(original->getMulticastGroup());
            for (size_t j = 0; j < original->getNumOutInterfaces(); j++)
                clone->addOutInterface(new IMulticastRoute::OutInterface(*original->getOutInterface(j)));
            node->routingTable->addMulticastRoute(clone);
        }
    }
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
    for (int bitIndex = sizeof(value) * 8 - 1; bitIndex >= 0; bitIndex--) {
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
    for (int bitIndex = 0; bitIndex < ADDRLEN_BITS; bitIndex++) {
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
    for (int valueIndex = 0; valueIndex < ADDRLEN_BITS; valueIndex++) {
        uint32 valueBitMask = (uint32)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0) {
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
    for (int valueIndex = 0; valueIndex < ADDRLEN_BITS; valueIndex++) {
        uint32 valueBitMask = (uint32)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0) {
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

bool Ipv4NetworkConfigurator::compareInterfaceInfos(InterfaceInfo *i, InterfaceInfo *j)
{
    return i->addressSpecifiedBits > j->addressSpecifiedBits;
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
void Ipv4NetworkConfigurator::collectCompatibleInterfaces(const std::vector<InterfaceInfo *>& interfaces,    /*in*/
        std::vector<Ipv4NetworkConfigurator::InterfaceInfo *>& compatibleInterfaces,    /*out, and the rest too*/
        uint32& mergedAddress, uint32& mergedAddressSpecifiedBits, uint32& mergedAddressIncompatibleBits,
        uint32& mergedNetmask, uint32& mergedNetmaskSpecifiedBits, uint32& mergedNetmaskIncompatibleBits)
{
    ASSERT(compatibleInterfaces.empty());
    mergedAddress = mergedAddressSpecifiedBits = mergedAddressIncompatibleBits = 0;
    mergedNetmask = mergedNetmaskSpecifiedBits = mergedNetmaskIncompatibleBits = 0;

    for (auto & interface : interfaces) {
        Ipv4NetworkConfigurator::InterfaceInfo *candidateInterface = interface;
        InterfaceEntry *ie = candidateInterface->interfaceEntry;

        // extract candidate interface configuration data
        uint32 candidateAddress = candidateInterface->address;
        uint32 candidateAddressSpecifiedBits = candidateInterface->addressSpecifiedBits;
        uint32 candidateNetmask = candidateInterface->netmask;
        uint32 candidateNetmaskSpecifiedBits = candidateInterface->netmaskSpecifiedBits;
        EV_TRACE << "Trying to merge " << ie->getInterfaceFullPath() << " interface with address specification: " << Ipv4Address(candidateAddress) << " / " << Ipv4Address(candidateAddressSpecifiedBits) << endl;
        EV_TRACE << "Trying to merge " << ie->getInterfaceFullPath() << " interface with netmask specification: " << Ipv4Address(candidateNetmask) << " / " << Ipv4Address(candidateNetmaskSpecifiedBits) << endl;

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
        EV_TRACE << "Merged address specification: " << Ipv4Address(mergedAddress) << " / " << Ipv4Address(mergedAddressSpecifiedBits) << " / " << Ipv4Address(mergedAddressIncompatibleBits) << endl;
        EV_TRACE << "Merged netmask specification: " << Ipv4Address(mergedNetmask) << " / " << Ipv4Address(mergedNetmaskSpecifiedBits) << " / " << Ipv4Address(mergedNetmaskIncompatibleBits) << endl;
    }
    // sort compatibleInterfaces moving the most constrained interfaces first
    // (stable sort tp garantee identical order if the interfaces are similarly constrained)
    std::stable_sort(compatibleInterfaces.begin(), compatibleInterfaces.end(), compareInterfaceInfos);
    EV_TRACE << "Found " << compatibleInterfaces.size() << " compatible interfaces" << endl;
}

void Ipv4NetworkConfigurator::assignAddresses(Topology& topology)
{
    if (configureIsolatedNetworksSeparatly) {
        std::map<int, std::vector<LinkInfo *>> isolatedNetworks;
        for (auto & linkInfo : topology.linkInfos) {
            int networkId = linkInfo->networkId;
            auto network = isolatedNetworks.find(networkId);
            if (network == isolatedNetworks.end()) {
                std::vector<LinkInfo *> collection = {linkInfo};
                isolatedNetworks[networkId] = collection;
            }
            else
                network->second.push_back(linkInfo);
        }
        for (auto & it : isolatedNetworks) {
            EV_DEBUG << "--> configuring isolated network " << it.first << ". \n";
            assignAddresses(it.second);
            EV_DEBUG << "<-- configuring isolated network " << it.first << ". \n";
        }
    }
    else
        assignAddresses(topology.linkInfos);
}

void Ipv4NetworkConfigurator::assignAddresses(std::vector<LinkInfo *> links)
{
    int bitSize = sizeof(uint32) * 8;
    std::vector<uint32> assignedNetworkAddresses;
    std::vector<uint32> assignedNetworkNetmasks;
    std::vector<uint32> assignedInterfaceAddresses;
    std::map<uint32, InterfaceEntry *> assignedAddressToInterfaceEntryMap;

    // iterate through all links and process them separately one by one
    for (auto & selectedLink : links) {
        std::vector<InterfaceInfo *> unconfiguredInterfaces;
        for (auto & element : selectedLink->interfaceInfos)
            unconfiguredInterfaces.push_back(static_cast<InterfaceInfo *>(element));
        // repeat until all interfaces of the selected link become configured
        // and assign addresses to groups of interfaces having compatible address and netmask specifications
        while (unconfiguredInterfaces.size() != 0) {
            // STEP 1.
            uint32 mergedAddress;    // compatible bits of the merged address (both 0 and 1 are address bits)
            uint32 mergedAddressSpecifiedBits;    // mask for the valid compatible bits of the merged address (0 means unspecified, 1 means specified)
            uint32 mergedAddressIncompatibleBits;    // incompatible bits of the merged address (0 means compatible, 1 means incompatible)
            uint32 mergedNetmask;    // compatible bits of the merged netmask (both 0 and 1 are netmask bits)
            uint32 mergedNetmaskSpecifiedBits;    // mask for the compatible bits of the merged netmask (0 means unspecified, 1 means specified)
            uint32 mergedNetmaskIncompatibleBits;    // incompatible bits of the merged netmask (0 means compatible, 1 means incompatible)
            std::vector<InterfaceInfo *> compatibleInterfaces;    // the list of compatible interfaces
            collectCompatibleInterfaces(unconfiguredInterfaces, compatibleInterfaces, mergedAddress, mergedAddressSpecifiedBits, mergedAddressIncompatibleBits, mergedNetmask, mergedNetmaskSpecifiedBits, mergedNetmaskIncompatibleBits);

            // STEP 2.
            // determine the valid range of netmask length by searching from left to right the last 1 and the first 0 bits
            // also consider the incompatible bits of the address to limit the range of valid netmasks accordingly
            int minimumNetmaskLength = bitSize - getLeastSignificantBitIndex(mergedNetmask & mergedNetmaskSpecifiedBits, 1, bitSize);    // 0 means 0.0.0.0, bitSize means 255.255.255.255
            int maximumNetmaskLength = bitSize - 1 - getMostSignificantBitIndex(~mergedNetmask & mergedNetmaskSpecifiedBits, 1, -1);    // 0 means 0.0.0.0, bitSize means 255.255.255.255
            maximumNetmaskLength = std::min(maximumNetmaskLength, bitSize - 1 - getMostSignificantBitIndex(mergedAddressIncompatibleBits, 1, -1));

            // make sure there are enough bits to configure a unique address for all interface
            // the +2 means that all-0 and all-1 addresses are ruled out
            int compatibleInterfaceCount = compatibleInterfaces.size() + 2;
            int interfaceAddressBitCount = getRepresentationBitCount(compatibleInterfaceCount);
            maximumNetmaskLength = std::min(maximumNetmaskLength, bitSize - interfaceAddressBitCount);
            EV_TRACE << "Netmask valid length range: " << minimumNetmaskLength << " - " << maximumNetmaskLength << endl;

            // STEP 3.
            // determine network address and network netmask by iterating through valid netmasks from longest to shortest
            int netmaskLength = -1;
            uint32 networkAddress = 0;    // network part of the addresses  (e.g. 10.1.1.0)
            uint32 networkNetmask = 0;    // netmask for the network (e.g. 255.255.255.0)
            ASSERT(maximumNetmaskLength < bitSize);
            for (netmaskLength = maximumNetmaskLength; netmaskLength >= minimumNetmaskLength; netmaskLength--) {
                ASSERT(netmaskLength < bitSize);
                networkNetmask = ~(~((uint32)0) >> netmaskLength);
                EV_TRACE << "Trying network netmask: " << Ipv4Address(networkNetmask) << " : " << netmaskLength << endl;
                networkAddress = mergedAddress & mergedAddressSpecifiedBits & networkNetmask;
                uint32 networkAddressUnspecifiedBits = ~mergedAddressSpecifiedBits & networkNetmask;    // 1 means the network address unspecified
                uint32 networkAddressUnspecifiedPartLimit = getPackedBits(~(uint32)0, networkAddressUnspecifiedBits) + (uint32)1;
                EV_TRACE << "Counting from: " << 0 << " to: " << networkAddressUnspecifiedPartLimit << endl;

                // we start with +1 so that the network address will be more likely different
                for (uint32 networkAddressUnspecifiedPart = 0; networkAddressUnspecifiedPart <= networkAddressUnspecifiedPartLimit; networkAddressUnspecifiedPart++) {
                    networkAddress = setPackedBits(networkAddress, networkAddressUnspecifiedBits, networkAddressUnspecifiedPart);
                    EV_TRACE << "Trying network address: " << Ipv4Address(networkAddress) << endl;
                    uint32 networkAddressMaximum = networkAddress | ~networkNetmask;

                    // check for overlapping network address ranges
                    if (assignDisjunctSubnetAddressesParameter) {
                        bool overlaps = false;
                        for (int i = 0; i < (int)assignedNetworkAddresses.size(); i++) {
                            uint32 assignedNetworkAddress = assignedNetworkAddresses[i];
                            uint32 assignedNetworkNetmask = assignedNetworkNetmasks[i];
                            uint32 assignedNetworkAddressMaximum = assignedNetworkAddress | ~assignedNetworkNetmask;
                            if (networkAddress <= assignedNetworkAddressMaximum && assignedNetworkAddress <= networkAddressMaximum)
                                overlaps = true;
                        }
                        if (overlaps)
                            continue;
                    }

                    // count interfaces that have the same address prefix
                    int interfaceCount = 0;
                    for (auto & assignedInterfaceAddress : assignedInterfaceAddresses)
                        if ((assignedInterfaceAddress & networkNetmask) == networkAddress)
                            interfaceCount++;

                    if (assignDisjunctSubnetAddressesParameter && interfaceCount != 0)
                        continue;
                    EV_TRACE << "Matching interface count: " << interfaceCount << endl;

                    // check if there's enough room for the interface addresses
                    if ((1 << (bitSize - netmaskLength)) >= interfaceCount + compatibleInterfaceCount)
                        goto found;
                }
            }
            found: if (netmaskLength < minimumNetmaskLength || netmaskLength > maximumNetmaskLength)
                throw cRuntimeError("Failed to find address prefix (using %s with specified bits %s) and netmask (length from %d bits to %d bits) for interface %s and %lu other interface(s). Please refine your parameters and try again!",
                        Ipv4Address(mergedAddress).str().c_str(), Ipv4Address(mergedAddressSpecifiedBits).str().c_str(), minimumNetmaskLength, maximumNetmaskLength,
                        compatibleInterfaces[0]->interfaceEntry->getInterfaceFullPath().c_str(), compatibleInterfaces.size() - 1);
            EV_TRACE << "Selected netmask length: " << netmaskLength << endl;
            EV_TRACE << "Selected network address: " << Ipv4Address(networkAddress) << endl;
            EV_TRACE << "Selected network netmask: " << Ipv4Address(networkNetmask) << endl;

            // STEP 4.
            // determine the complete IP address for all compatible interfaces
            for (auto & compatibleInterface : compatibleInterfaces) {
                InterfaceEntry *interfaceEntry = compatibleInterface->interfaceEntry;
                uint32 interfaceAddress = compatibleInterface->address & ~networkNetmask;
                uint32 interfaceAddressSpecifiedBits = compatibleInterface->addressSpecifiedBits;
                uint32 interfaceAddressUnspecifiedBits = ~interfaceAddressSpecifiedBits & ~networkNetmask;    // 1 means the interface address is unspecified
                uint32 interfaceAddressUnspecifiedPartMaximum = 0;
                for (auto & assignedInterfaceAddress : assignedInterfaceAddresses) {
                    uint32 otherInterfaceAddress = assignedInterfaceAddress;
                    if ((otherInterfaceAddress & ~interfaceAddressUnspecifiedBits) == ((networkAddress | interfaceAddress) & ~interfaceAddressUnspecifiedBits)) {
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
                if (assignUniqueAddresses && assignedAddressToInterfaceEntryMap.find(completeAddress) != assignedAddressToInterfaceEntryMap.end())
                    throw cRuntimeError("Failed to configure unique address for %s. Please refine your parameters and try again!", interfaceEntry->getInterfaceFullPath().c_str());
                assignedAddressToInterfaceEntryMap[completeAddress] = compatibleInterface->interfaceEntry;
                assignedInterfaceAddresses.push_back(completeAddress);

                // configure interface with the selected address and netmask
                EV_DEBUG << "Setting interface address, interface = " << compatibleInterface->getFullPath() << ", address = " << Ipv4Address(completeAddress) << ", netmask = " << Ipv4Address(completeNetmask) << endl;
                compatibleInterface->address = completeAddress;
                compatibleInterface->addressSpecifiedBits = 0xFFFFFFFF;
                compatibleInterface->netmask = completeNetmask;
                compatibleInterface->netmaskSpecifiedBits = 0xFFFFFFFF;

                // remove configured interface
                unconfiguredInterfaces.erase(find(unconfiguredInterfaces, compatibleInterface));
            }

            // register the network address and netmask as being used
            assignedNetworkAddresses.push_back(networkAddress);
            assignedNetworkNetmasks.push_back(networkNetmask);
        }
    }
}

Ipv4NetworkConfigurator::InterfaceInfo *Ipv4NetworkConfigurator::createInterfaceInfo(NetworkConfiguratorBase::Topology& topology, NetworkConfiguratorBase::Node *node, LinkInfo *linkInfo, InterfaceEntry *ie)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(static_cast<Ipv4NetworkConfigurator::Node *>(node), linkInfo, ie);
    Ipv4InterfaceData *ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
    if (ipv4Data) {
        Ipv4Address address = ipv4Data->getIPAddress();
        Ipv4Address netmask = ipv4Data->getNetmask();
        if (!address.isUnspecified()) {
            interfaceInfo->address = address.getInt();
            interfaceInfo->addressSpecifiedBits = 0xFFFFFFFF;
            interfaceInfo->netmask = netmask.getInt();
            interfaceInfo->netmaskSpecifiedBits = 0xFFFFFFFF;
        }
    }
    node->interfaceInfos.push_back(interfaceInfo);
    topology.interfaceInfos[ie->getId()] = interfaceInfo;
    return interfaceInfo;
}

void Ipv4NetworkConfigurator::readInterfaceConfiguration(Topology& topology)
{
    using namespace xmlutils;

    std::set<InterfaceInfo *> interfacesSeen;
    cXMLElementList interfaceElements = configuration->getChildrenByTagName("interface");

    for (auto & interfaceElement : interfaceElements) {
        const char *hostAttr = interfaceElement->getAttribute("hosts");    // "host* router[0..3]"
        const char *interfaceAttr = interfaceElement->getAttribute("names");    // i.e. interface names, like "eth* ppp0"

        // TODO: "switch" egyebkent sztem nem muxik most, de kellene!
        const char *towardsAttr = interfaceElement->getAttribute("towards");    // neighbor host names, like "ap switch"
        const char *amongAttr = interfaceElement->getAttribute("among");    // neighbor host names, like "host[*] router1"
        const char *addressAttr = interfaceElement->getAttribute("address");    // "10.0.x.x"
        const char *netmaskAttr = interfaceElement->getAttribute("netmask");    // "255.255.x.x"
        const char *mtuAttr = interfaceElement->getAttribute("mtu");    // integer
        const char *metricAttr = interfaceElement->getAttribute("metric");    // integer
        const char *groupsAttr = interfaceElement->getAttribute("groups");    // list of multicast addresses
        bool addStaticRouteAttr = getAttributeBoolValue(interfaceElement, "add-static-route", true);
        bool addDefaultRouteAttr = getAttributeBoolValue(interfaceElement, "add-default-route", true);
        bool addSubnetRouteAttr = getAttributeBoolValue(interfaceElement, "add-subnet-route", true);

        if (amongAttr && *amongAttr) {    // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", interfaceElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try {
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
            if (haveNetmaskConstraint) {
                if (netmaskAttr[0] == '/') {
                    netmask = Ipv4Address::makeNetmask(atoi(netmaskAttr + 1)).getInt();
                    netmaskSpecifiedBits = 0xffffffffLU;
                }
                else
                    parseAddressAndSpecifiedBits(netmaskAttr, netmask, netmaskSpecifiedBits);
            }

            // configure address/netmask constraints on matching interfaces
            for (auto & linkInfo : topology.linkInfos) {
                for (int j = 0; j < (int)linkInfo->interfaceInfos.size(); j++) {
                    InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(linkInfo->interfaceInfos[j]);
                    if (interfacesSeen.count(interfaceInfo) == 0) {
                        cModule *hostModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
                        std::string hostFullPath = hostModule->getFullPath();
                        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                        // Note: "hosts", "interfaces" and "towards" must ALL match on the interface for the rule to apply
                        if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                            (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceInfo->interfaceEntry->getInterfaceName())) &&
                            (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(linkInfo, &towardsMatcher, hostModule)))
                        {
                            EV_DEBUG << "Processing interface configuration for " << interfaceInfo->getFullPath() << endl;

                            // unicast address constraints
                            interfaceInfo->configure = haveAddressConstraint;
                            if (interfaceInfo->configure) {
                                interfaceInfo->address = address;
                                interfaceInfo->addressSpecifiedBits = addressSpecifiedBits;
                                if (haveNetmaskConstraint) {
                                    interfaceInfo->netmask = netmask;
                                    interfaceInfo->netmaskSpecifiedBits = netmaskSpecifiedBits;
                                }
                            }

                            // route flags
                            interfaceInfo->addStaticRoute = addStaticRouteAttr;
                            interfaceInfo->addDefaultRoute = addDefaultRouteAttr;
                            interfaceInfo->addSubnetRoute = addSubnetRouteAttr;

                            // mtu
                            if (isNotEmpty(mtuAttr))
                                interfaceInfo->mtu = atoi(mtuAttr);

                            // metric
                            if (isNotEmpty(metricAttr))
                                interfaceInfo->metric = atoi(metricAttr);

                            // groups
                            if (isNotEmpty(groupsAttr)) {
                                cStringTokenizer tokenizer(groupsAttr);
                                while (tokenizer.hasMoreTokens())
                                    interfaceInfo->multicastGroups.push_back(Ipv4Address(tokenizer.nextToken()));
                            }

                            interfacesSeen.insert(interfaceInfo);
                        }
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <interface> element at %s: %s", interfaceElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv4NetworkConfigurator::parseAddressAndSpecifiedBits(const char *addressAttr, uint32_t& outAddress, uint32_t& outAddressSpecifiedBits)
{
    // change "10.0.x.x" to "10.0.0.0" (for address) and "255.255.0.0" (for specifiedBits)
    std::string address;
    std::string specifiedBits;
    cStringTokenizer tokenizer(addressAttr, ".");
    while (tokenizer.hasMoreTokens()) {
        std::string token = tokenizer.nextToken();
        address += (token == "x") ? "0." : (token + ".");
        specifiedBits += (token == "x") ? "0." : "255.";
    }
    address = address.substr(0, address.size() - 1);
    specifiedBits = specifiedBits.substr(0, specifiedBits.size() - 1);

    if (!Ipv4Address::isWellFormed(address.c_str()) || !Ipv4Address::isWellFormed(specifiedBits.c_str()))
        throw cRuntimeError("Malformed Ipv4 address or netmask constraint '%s'", addressAttr);

    outAddress = Ipv4Address(address.c_str()).getInt();
    outAddressSpecifiedBits = Ipv4Address(specifiedBits.c_str()).getInt();
}

bool Ipv4NetworkConfigurator::linkContainsMatchingHostExcept(LinkInfo *linkInfo, Matcher *hostMatcher, cModule *exceptModule)
{
    for (auto & element : linkInfo->interfaceInfos) {
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
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

void Ipv4NetworkConfigurator::dumpLinks(Topology& topology)
{
    for (size_t i = 0; i < topology.linkInfos.size(); i++) {
        EV_INFO << "Link " << i << endl;
        LinkInfo *linkInfo = topology.linkInfos[i];
        for (auto & element : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
            EV_INFO << "     " << interfaceInfo->interfaceEntry->getInterfaceFullPath() << endl;
        }
    }
}

void Ipv4NetworkConfigurator::dumpAddresses(Topology& topology)
{
    for (size_t i = 0; i < topology.linkInfos.size(); i++) {
        EV_INFO << "Link " << i << endl;
        LinkInfo *linkInfo = topology.linkInfos[i];
        for (auto & interfaceInfo : linkInfo->interfaceInfos) {
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            cModule *host = interfaceInfo->node->module;
            EV_INFO << "    " << host->getFullName() << " / " << interfaceEntry->str() << endl;
        }
    }
}

void Ipv4NetworkConfigurator::dumpRoutes(Topology& topology)
{
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        if (node->routingTable) {
            EV_INFO << "Node " << node->module->getFullPath() << endl;
            check_and_cast<IIpv4RoutingTable *>(node->routingTable)->printRoutingTable();
            if (node->routingTable->getNumMulticastRoutes() > 0)
                check_and_cast<IIpv4RoutingTable *>(node->routingTable)->printMulticastRoutingTable();
        }
    }
}

void Ipv4NetworkConfigurator::dumpConfig(Topology& topology)
{
    FILE *f;
    const char *filename = par("dumpConfig");
    inet::utils::makePathForFile(filename);
    f = fopen(filename, "w");
    if (!f)
        throw cRuntimeError("Cannot write configurator output file");
    fprintf(f, "<config>\n");

    // interfaces
    for (auto & linkInfo : topology.linkInfos) {
        for (auto & element : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            Ipv4InterfaceData *interfaceData = interfaceEntry->getProtocolData<Ipv4InterfaceData>();
            std::stringstream stream;
            stream << "   <interface hosts=\"" << interfaceInfo->node->module->getFullPath() << "\" names=\"" << interfaceEntry->getInterfaceName()
                   << "\" address=\"" << interfaceData->getIPAddress() << "\" netmask=\"" << interfaceData->getNetmask()
                   << "\" metric=\"" << interfaceData->getMetric()
                   << "\"/>" << endl;
            fprintf(f, "%s", stream.str().c_str());
        }
    }

    // multicast groups
    for (auto & linkInfo : topology.linkInfos) {
        for (auto & element : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
            InterfaceEntry *interfaceEntry = interfaceInfo->interfaceEntry;
            Ipv4InterfaceData *interfaceData = interfaceEntry->getProtocolData<Ipv4InterfaceData>();
            int numOfMulticastGroups = interfaceData->getNumOfJoinedMulticastGroups();
            if (numOfMulticastGroups > 0) {
                std::stringstream stream;
                stream << "   <multicast-group hosts=\"" << interfaceInfo->node->module->getFullPath() << "\" interfaces=\"" << interfaceEntry->getInterfaceName() << "\" address=\"";
                for (int k = 0; k < numOfMulticastGroups; k++) {
                    Ipv4Address address = interfaceData->getJoinedMulticastGroup(k);
                    if (k)
                        stream << " ";
                    stream << address.str();
                }
                stream << "\"/>" << endl;
                fprintf(f, "%s", stream.str().c_str());
            }
        }
    }

    // wireless links
    for (auto & linkInfo: topology.linkInfos) {
        bool hasWireless = false;
        for (auto & element : linkInfo->interfaceInfos) {
            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
            if (isWirelessInterface(interfaceInfo->interfaceEntry))
                hasWireless = true;
        }
        if (hasWireless) {
            bool first = true;
            std::stringstream stream;
            stream << "   <wireless interfaces=\"";
            for (auto & element : linkInfo->interfaceInfos) {
                if (!first)
                    stream << " ";
                InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
                if (isWirelessInterface(interfaceInfo->interfaceEntry)) {
                    stream << interfaceInfo->node->module->getFullPath() << "%" << interfaceInfo->interfaceEntry->getInterfaceName();
                    first = false;
                }
            }
            stream << "\"/>" << endl;
            fprintf(f, "%s", stream.str().c_str());
        }
    }

    // routes
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        IIpv4RoutingTable *routingTable = dynamic_cast<IIpv4RoutingTable *>(node->routingTable);
        if (routingTable) {
            for (int j = 0; j < routingTable->getNumRoutes(); j++) {
                Ipv4Route *route = routingTable->getRoute(j);
                std::stringstream stream;
                Ipv4Address netmask = route->getNetmask();
                Ipv4Address gateway = route->getGateway();
                stream << "   <route hosts=\"" << node->module->getFullPath();
                stream << "\" destination=\"";
                if (route->getDestination().isUnspecified())
                    stream << "*";
                else
                    stream << route->getDestination();
                stream << "\" netmask=\"";
                if (route->getNetmask().isUnspecified())
                    stream << "*";
                else
                    stream << route->getNetmask();
                stream << "\" gateway=\"";
                if (route->getGateway().isUnspecified())
                    stream << "*";
                else
                    stream << route->getGateway();
                stream << "\" interface=\"" << route->getInterfaceName() << "\" metric=\"" << route->getMetric() << "\"/>" << endl;
                fprintf(f, "%s", stream.str().c_str());
            }
        }
    }

    // multicast routes
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        IIpv4RoutingTable *routingTable = dynamic_cast<IIpv4RoutingTable *>(node->routingTable);
        if (routingTable) {
            for (int j = 0; j < routingTable->getNumMulticastRoutes(); j++) {
                Ipv4MulticastRoute *route = routingTable->getMulticastRoute(j);
                std::stringstream stream;
                stream << "   <multicast-route hosts=\"" << node->module->getFullPath();
                stream << "\" source=\"";
                if (route->getOrigin().isUnspecified())
                    stream << "*";
                else
                    stream << route->getOrigin();
                stream << "\" netmask=\"";
                if (route->getOriginNetmask().isUnspecified())
                    stream << "*";
                else
                    stream << route->getOriginNetmask();
                stream << "\" groups=\"";
                if (route->getMulticastGroup().isUnspecified())
                    stream << "*";
                else
                    stream << route->getMulticastGroup();
                if (route->getInInterface())
                    stream << "\" parent=\"" << route->getInInterface()->getInterface()->getInterfaceName();
                stream << "\" children=\"";
                for (unsigned int k = 0; k < route->getNumOutInterfaces(); k++) {
                    if (k)
                        stream << " ";
                    stream << route->getOutInterface(k)->getInterface()->getInterfaceName();
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

void Ipv4NetworkConfigurator::readMulticastGroupConfiguration(Topology& topology)
{
    cXMLElementList multicastGroupElements = configuration->getChildrenByTagName("multicast-group");
    for (auto & multicastGroupElement : multicastGroupElements) {
        const char *hostAttr = multicastGroupElement->getAttribute("hosts");
        const char *interfaceAttr = multicastGroupElement->getAttribute("interfaces");
        const char *addressAttr = multicastGroupElement->getAttribute("address");
        const char *towardsAttr = multicastGroupElement->getAttribute("towards");    // neighbor host names, like "ap switch"
        const char *amongAttr = multicastGroupElement->getAttribute("among");

        if (amongAttr && *amongAttr) {    // among="X Y Z" means hosts = "X Y Z" towards = "X Y Z"
            if ((hostAttr && *hostAttr) || (towardsAttr && *towardsAttr))
                throw cRuntimeError("The 'hosts'/'towards' and 'among' attributes are mutually exclusive, at %s", multicastGroupElement->getSourceLocation());
            towardsAttr = hostAttr = amongAttr;
        }

        try {
            Matcher hostMatcher(hostAttr);
            Matcher interfaceMatcher(interfaceAttr);
            Matcher towardsMatcher(towardsAttr);

            // parse group addresses
            std::vector<Ipv4Address> multicastGroups;
            cStringTokenizer tokenizer(addressAttr);
            while (tokenizer.hasMoreTokens()) {
                Ipv4Address addr = Ipv4Address(tokenizer.nextToken());
                if (!addr.isMulticast())
                    throw cRuntimeError("Non-multicast address %s found in the multicast-group element", addr.str().c_str());
                multicastGroups.push_back(addr);
            }

            for (auto & linkInfo : topology.linkInfos) {
                for (size_t k = 0; k < linkInfo->interfaceInfos.size(); k++) {
                    InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(linkInfo->interfaceInfos[k]);
                    cModule *hostModule = interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule();
                    std::string hostFullPath = hostModule->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);

                    if ((hostMatcher.matchesAny() || hostMatcher.matches(hostShortenedFullPath.c_str()) || hostMatcher.matches(hostFullPath.c_str())) &&
                        (interfaceMatcher.matchesAny() || interfaceMatcher.matches(interfaceInfo->interfaceEntry->getInterfaceName())) &&
                        (towardsMatcher.matchesAny() || linkContainsMatchingHostExcept(linkInfo, &towardsMatcher, hostModule)))
                    {
                        for (auto & multicastGroup : multicastGroups)
                            interfaceInfo->multicastGroups.push_back(multicastGroup);
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <multicast-group> element at %s: %s", multicastGroupElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv4NetworkConfigurator::readManualRouteConfiguration(Topology& topology)
{
    cXMLElementList routeElements = configuration->getChildrenByTagName("route");
    for (auto & routeElement : routeElements) {
        const char *hostAttr = xmlutils::getMandatoryFilledAttribute(*routeElement, "hosts");
        const char *destinationAttr = xmlutils::getMandatoryAttribute(*routeElement, "destination");    // destination address  (L3AddressResolver syntax)
        const char *netmaskAttr = routeElement->getAttribute("netmask");    // default: 255.255.255.255; alternative notation: "/23"
        const char *gatewayAttr = routeElement->getAttribute("gateway");    // next hop address (L3AddressResolver syntax)
        const char *interfaceAttr = routeElement->getAttribute("interface");    // output interface name
        const char *metricAttr = routeElement->getAttribute("metric");

        try {
            // parse and check the attributes
            Ipv4Address destination;
            if (!isEmpty(destinationAttr) && strcmp(destinationAttr, "*"))
                destination = resolve(destinationAttr, L3AddressResolver::ADDR_IPv4).toIpv4();
            Ipv4Address netmask;
            if (!isEmpty(netmaskAttr) && strcmp(netmaskAttr, "*")) {
                if (netmaskAttr[0] == '/')
                    netmask = Ipv4Address::makeNetmask(atoi(netmaskAttr + 1));
                else
                    netmask = Ipv4Address(netmaskAttr);
            }
            if (!netmask.isValidNetmask())
                throw cRuntimeError("Wrong netmask %s", netmask.str().c_str());
            if (isEmpty(interfaceAttr) && isEmpty(gatewayAttr))
                throw cRuntimeError("Incomplete route: either gateway or interface (or both) must be specified");

            // find matching host(s), and add the route
            Matcher atMatcher(hostAttr);
            for (int i = 0; i < topology.getNumNodes(); i++) {
                Node *node = (Node *)topology.getNode(i);
                if (node->routingTable) {
                    std::string hostFullPath = node->module->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                    if (atMatcher.matches(hostShortenedFullPath.c_str()) || atMatcher.matches(hostFullPath.c_str())) {
                        // determine the gateway (its address towards this node!) and the output interface for the route (must be done per node)
                        InterfaceEntry *ie;
                        Ipv4Address gateway;
                        resolveInterfaceAndGateway(node, interfaceAttr, gatewayAttr, ie, gateway, topology);

                        // create and add route
                        Ipv4Route *route = new Ipv4Route();
                        route->setSourceType(IRoute::MANUAL);
                        route->setDestination(destination);
                        route->setNetmask(netmask);
                        route->setGateway(gateway);    // may be unspecified
                        route->setInterface(ie);
                        if (isNotEmpty(metricAttr))
                            route->setMetric(atoi(metricAttr));
                        node->staticRoutes.push_back(route);
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv4NetworkConfigurator::readManualMulticastRouteConfiguration(Topology& topology)
{
    cXMLElementList routeElements = configuration->getChildrenByTagName("multicast-route");
    for (auto & routeElement : routeElements) {
        const char *hostAttr = routeElement->getAttribute("hosts");
        const char *sourceAttr = routeElement->getAttribute("source");    // source address  (L3AddressResolver syntax)
        const char *netmaskAttr = routeElement->getAttribute("netmask");    // default: 255.255.255.255; alternative notation: "/23"
        const char *groupsAttr = routeElement->getAttribute("groups");    // addresses of the multicast groups, default: 0.0.0.0, matching all groups
        const char *parentAttr = routeElement->getAttribute("parent");    // name of expected input interface
        const char *childrenAttr = routeElement->getAttribute("children");    // names of output interfaces
        const char *metricAttr = routeElement->getAttribute("metric");

        try {
            // parse and check the attributes
            Ipv4Address source;
            if (!isEmpty(sourceAttr) && strcmp(sourceAttr, "*"))
                source = resolve(sourceAttr, L3AddressResolver::ADDR_IPv4).toIpv4();
            Ipv4Address netmask;
            if (!isEmpty(netmaskAttr) && strcmp(netmaskAttr, "*")) {
                if (netmaskAttr[0] == '/')
                    netmask = Ipv4Address::makeNetmask(atoi(netmaskAttr + 1));
                else
                    netmask = Ipv4Address(netmaskAttr);
            }

            if (!netmask.isValidNetmask())
                throw cRuntimeError("Wrong netmask %s", netmask.str().c_str());

            std::vector<Ipv4Address> groups;
            if (isEmpty(groupsAttr))
                groups.push_back(Ipv4Address::UNSPECIFIED_ADDRESS);
            else {
                cStringTokenizer tokenizer(groupsAttr);
                while (tokenizer.hasMoreTokens()) {
                    Ipv4Address group = Ipv4Address(tokenizer.nextToken());
                    if (!group.isMulticast())
                        throw cRuntimeError("Address '%s' in groups attribute is not multicast.", group.str().c_str());
                    groups.push_back(group);
                }
            }

            // find matching host(s), and add the route
            Matcher atMatcher(hostAttr);
            InterfaceMatcher childrenMatcher(childrenAttr);
            for (int i = 0; i < topology.getNumNodes(); i++) {
                Node *node = (Node *)topology.getNode(i);
                if (node->routingTable && node->routingTable->isMulticastForwardingEnabled()) {
                    std::string hostFullPath = node->module->getFullPath();
                    std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
                    if (atMatcher.matches(hostShortenedFullPath.c_str()) || atMatcher.matches(hostFullPath.c_str())) {
                        InterfaceEntry *parent = nullptr;
                        if (!isEmpty(parentAttr)) {
                            parent = node->interfaceTable->findInterfaceByName(parentAttr);
                            if (!parent)
                                throw cRuntimeError("Parent interface '%s' not found.", parentAttr);
                            if (!parent->isMulticast())
                                throw cRuntimeError("Parent interface '%s' is not multicast.", parentAttr);
                        }

                        std::vector<InterfaceEntry *> children;
                        for (auto & element : node->interfaceInfos) {
                            InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
                            InterfaceEntry *ie = interfaceInfo->interfaceEntry;
                            if (ie != parent && ie->isMulticast() && childrenMatcher.matches(interfaceInfo))
                                children.push_back(ie);
                        }

                        for (auto & group : groups) {
                            // create and add route
                            Ipv4MulticastRoute *route = new Ipv4MulticastRoute();
                            route->setSourceType(IMulticastRoute::MANUAL);
                            route->setOrigin(source);
                            route->setOriginNetmask(netmask);
                            route->setMulticastGroup(group);
                            route->setInInterface(parent ? new Ipv4MulticastRoute::InInterface(parent) : nullptr);
                            if (isNotEmpty(metricAttr))
                                route->setMetric(atoi(metricAttr));
                            for (auto & child : children)
                                route->addOutInterface(new Ipv4MulticastRoute::OutInterface(child, false    /*TODO:isLeaf*/));
                            node->staticMulticastRoutes.push_back(route);
                        }
                    }
                }
            }
        }
        catch (std::exception& e) {
            throw cRuntimeError("Error in XML <multicast-route> element at %s: %s", routeElement->getSourceLocation(), e.what());
        }
    }
}

void Ipv4NetworkConfigurator::resolveInterfaceAndGateway(Node *node, const char *interfaceAttr, const char *gatewayAttr,
        InterfaceEntry *& outIE, Ipv4Address& outGateway, Topology& topology)
{
    // resolve interface name
    if (isEmpty(interfaceAttr)) {
        outIE = nullptr;
    }
    else {
        outIE = node->interfaceTable->findInterfaceByName(interfaceAttr);
        if (!outIE)
            throw cRuntimeError("Host/router %s has no interface named \"%s\"",
                    node->module->getFullPath().c_str(), interfaceAttr);
    }

    // if gateway is not specified, we are done
    if (isEmpty(gatewayAttr) || !strcmp(gatewayAttr, "*")) {
        outGateway = Ipv4Address();
        return;    // outInterface also already done -- we're done
    }

    ASSERT(isNotEmpty(gatewayAttr));    // see "if" above

    // check syntax of gatewayAttr, and obtain an initial value
    outGateway = resolve(gatewayAttr, L3AddressResolver::ADDR_IPv4).toIpv4();

    Ipv4Address gatewayAddressOnCommonLink;

    if (!outIE) {
        // interface is not specified explicitly -- we must deduce it from the gateway.
        // It is expected that the gateway is on the same link with the configured node,
        // and then we pick the interface which connects to that link.

        // loop through all links, and find the one that contains both the
        // configured node and the gateway
        for (auto & linkInfo : topology.linkInfos) {
            InterfaceInfo *gatewayInterfaceOnLink = findInterfaceOnLinkByNodeAddress(linkInfo, outGateway);
            if (gatewayInterfaceOnLink) {
                InterfaceInfo *nodeInterfaceOnLink = findInterfaceOnLinkByNode(linkInfo, node->module);
                if (nodeInterfaceOnLink) {
                    outIE = nodeInterfaceOnLink->interfaceEntry;
                    gatewayAddressOnCommonLink = gatewayInterfaceOnLink->getAddress();
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
    // in a syntax accepted by L3AddressResolver. If the gatewayAttr is a concrete IP address
    // or contains a gateway interface name (L3AddressResolver accepts it after a "/"), we're done
    if (Ipv4Address::isWellFormed(gatewayAttr) || strchr(gatewayAttr, '/') != nullptr)
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
            outGateway = gatewayInterface->getAddress();
    }
}

Ipv4NetworkConfigurator::InterfaceInfo *Ipv4NetworkConfigurator::findInterfaceOnLinkByNode(LinkInfo *linkInfo, cModule *node)
{
    for (auto & element : linkInfo->interfaceInfos) {
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
        if (interfaceInfo->interfaceEntry->getInterfaceTable()->getHostModule() == node)
            return interfaceInfo;
    }
    return nullptr;
}

Ipv4NetworkConfigurator::InterfaceInfo *Ipv4NetworkConfigurator::findInterfaceOnLinkByNodeAddress(LinkInfo *linkInfo, Ipv4Address address)
{
    for (auto & element : linkInfo->interfaceInfos) {
        // if the interface has this address, found
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(element);
        if (interfaceInfo->address == address.getInt())
            return interfaceInfo;

        // if some other interface of the same node has the address, we accept that too
        NetworkConfiguratorBase::Node *node = interfaceInfo->node;
        for (auto & it : node->interfaceInfos)
            if (static_cast<InterfaceInfo *>(it)->getAddress() == address)
                return interfaceInfo;

    }
    return nullptr;
}

Ipv4NetworkConfigurator::LinkInfo *Ipv4NetworkConfigurator::findLinkOfInterface(Topology& topology, InterfaceEntry *ie)
{
    for (auto & linkInfo : topology.linkInfos) {
        for (size_t j = 0; j < linkInfo->interfaceInfos.size(); j++)
            if (linkInfo->interfaceInfos[j]->interfaceEntry == ie)
                return linkInfo;
    }
    return nullptr;
}

IRoutingTable *Ipv4NetworkConfigurator::findRoutingTable(NetworkConfiguratorBase::Node *node)
{
    return L3AddressResolver().findIpv4RoutingTableOf(node->module);
}

bool Ipv4NetworkConfigurator::containsRoute(const std::vector<Ipv4Route *>& routes, Ipv4Route *route)
{
    for (auto & rt : routes)
        if (*rt == *route)
            return true;
    return false;
}

void Ipv4NetworkConfigurator::addStaticRoutes(Topology& topology, cXMLElement *autorouteElement)
{
    // set node weights
    const char *metric = autorouteElement->getAttribute("metric");
    if (metric == nullptr)
        metric = "hopCount";
    cXMLElement defaultNodeElement("node", "", nullptr);
    cXMLElementList nodeElements = autorouteElement->getChildrenByTagName("node");
    for (int i = 0; i < topology.getNumNodes(); i++) {
        cXMLElement *selectedNodeElement = &defaultNodeElement;
        Node *node = (Node *)topology.getNode(i);
        for (auto & nodeElement : nodeElements) {
            const char* hosts = nodeElement->getAttribute("hosts");
            if (hosts == nullptr)
                hosts = "**";
            Matcher nodeHostsMatcher(hosts);
            std::string hostFullPath = node->module->getFullPath();
            std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
            if (nodeHostsMatcher.matchesAny() || nodeHostsMatcher.matches(hostShortenedFullPath.c_str()) || nodeHostsMatcher.matches(hostFullPath.c_str())) {
                selectedNodeElement = nodeElement;
                break;
            }
        }
        double weight = computeNodeWeight(node, metric, selectedNodeElement);
        EV_DEBUG << "Setting node weight, node = " << node->module->getFullPath() << ", weight = " << weight << endl;
        node->setWeight(weight);
    }
    // set link weights
    cXMLElement defaultLinkElement("link", "", nullptr);
    cXMLElementList linkElements = autorouteElement->getChildrenByTagName("link");
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *node = (Node *)topology.getNode(i);
        for (int j = 0; j < node->getNumInLinks(); j++) {
            cXMLElement *selectedLinkElement = &defaultLinkElement;
            Link *link = (Link *)node->getLinkIn(j);
            for (auto & linkElement : linkElements) {
                const char* interfaces = linkElement->getAttribute("interfaces");
                if (interfaces == nullptr)
                    interfaces = "**";
                Matcher linkInterfaceMatcher(interfaces);
                std::string sourceFullPath = link->sourceInterfaceInfo->getFullPath();
                std::string sourceShortenedFullPath = sourceFullPath.substr(sourceFullPath.find('.') + 1);
                std::string destinationFullPath = link->destinationInterfaceInfo->getFullPath();
                std::string destinationShortenedFullPath = destinationFullPath.substr(destinationFullPath.find('.') + 1);
                if (linkInterfaceMatcher.matchesAny() ||
                    linkInterfaceMatcher.matches(sourceFullPath.c_str()) || linkInterfaceMatcher.matches(sourceShortenedFullPath.c_str()) ||
                    linkInterfaceMatcher.matches(destinationFullPath.c_str()) || linkInterfaceMatcher.matches(destinationShortenedFullPath.c_str()))
                {
                    selectedLinkElement = linkElement;
                    break;
                }
            }
            double weight = computeLinkWeight(link, metric, selectedLinkElement);
            EV_DEBUG << "Setting link weight, link = " << link << ", weight = " << weight << endl;
            link->setWeight(weight);
        }
    }
    // add static routes for all routing tables
    const char* sourceHosts = autorouteElement->getAttribute("sourceHosts");
    if (sourceHosts == nullptr)
        sourceHosts = "**";
    const char* destinationInterfaces = autorouteElement->getAttribute("destinationInterfaces");
    if (destinationInterfaces == nullptr)
        destinationInterfaces = "**";
    Matcher sourceHostsMatcher(sourceHosts);
    Matcher destinationInterfacesMatcher(destinationInterfaces);
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Node *sourceNode = (Node *)topology.getNode(i);
        std::string hostFullPath = sourceNode->module->getFullPath();
        std::string hostShortenedFullPath = hostFullPath.substr(hostFullPath.find('.') + 1);
        if (!sourceHostsMatcher.matchesAny() && !sourceHostsMatcher.matches(hostShortenedFullPath.c_str()) && !sourceHostsMatcher.matches(hostFullPath.c_str()))
            continue;
        if (isBridgeNode(sourceNode))
            continue;
        // calculate shortest paths from everywhere to sourceNode
        // we are going to use the paths in reverse direction (assuming all links are bidirectional)
        topology.calculateWeightedSingleShortestPathsTo(sourceNode);
        // check if adding the default routes would be ok (this is an optimization)
        if (addDefaultRoutesParameter && sourceNode->interfaceInfos.size() == 1 && sourceNode->interfaceInfos[0]->linkInfo->gatewayInterfaceInfo && sourceNode->interfaceInfos[0]->addDefaultRoute) {
            InterfaceInfo *sourceInterfaceInfo = static_cast<InterfaceInfo *>(sourceNode->interfaceInfos[0]);
            InterfaceEntry *sourceInterfaceEntry = sourceInterfaceInfo->interfaceEntry;
            InterfaceInfo *gatewayInterfaceInfo = static_cast<InterfaceInfo *>(sourceInterfaceInfo->linkInfo->gatewayInterfaceInfo);
            //InterfaceEntry *gatewayInterfaceEntry = gatewayInterfaceInfo->interfaceEntry;

            if (addDirectRoutesParameter) {
                // add a network route for the local network using ARP
                Ipv4Route *route = new Ipv4Route();
                route->setDestination(sourceInterfaceInfo->getAddress().doAnd(sourceInterfaceInfo->getNetmask()));
                route->setGateway(Ipv4Address::UNSPECIFIED_ADDRESS);
                route->setNetmask(sourceInterfaceInfo->getNetmask());
                route->setInterface(sourceInterfaceEntry);
                route->setSourceType(Ipv4Route::MANUAL);
                sourceNode->staticRoutes.push_back(route);
            }

            // add a default route towards the only one gateway
            Ipv4Route *route = new Ipv4Route();
            Ipv4Address gateway = gatewayInterfaceInfo->getAddress();
            route->setDestination(Ipv4Address::UNSPECIFIED_ADDRESS);
            route->setNetmask(Ipv4Address::UNSPECIFIED_ADDRESS);
            route->setGateway(gateway);
            route->setInterface(sourceInterfaceEntry);
            route->setSourceType(Ipv4Route::MANUAL);
            sourceNode->staticRoutes.push_back(route);

            // skip building and optimizing the whole routing table
            EV_DEBUG << "Adding default routes to " << sourceNode->getModule()->getFullPath() << ", node has only one (non-loopback) interface\n";
        }
        else {
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

                // determine next hop interface
                // find next hop interface (the last IP interface on the path that is not in the source node)
                Node *node = destinationNode;
                Link *link = nullptr;
                InterfaceInfo *nextHopInterfaceInfo = nullptr;
                while (node != sourceNode) {
                    link = (Link *)node->getPath(0);
                    if (node != sourceNode && !isBridgeNode(node) && link->sourceInterfaceInfo)
                        nextHopInterfaceInfo = static_cast<InterfaceInfo *>(link->sourceInterfaceInfo);
                    node = (Node *)node->getPath(0)->getRemoteNode();
                }

                // determine source interface
                if (nextHopInterfaceInfo && link->destinationInterfaceInfo && link->destinationInterfaceInfo->addStaticRoute) {
                    InterfaceEntry *sourceInterfaceEntry = link->destinationInterfaceInfo->interfaceEntry;
                    // add the same routes for all destination interfaces (IP packets are accepted from any interface at the destination)
                    for (size_t j = 0; j < destinationNode->interfaceInfos.size(); j++) {
                        InterfaceInfo *destinationInterfaceInfo = static_cast<InterfaceInfo *>(destinationNode->interfaceInfos[j]);
                        std::string destinationFullPath = destinationInterfaceInfo->interfaceEntry->getInterfaceFullPath();
                        std::string destinationShortenedFullPath = destinationFullPath.substr(destinationFullPath.find('.') + 1);
                        if (!destinationInterfacesMatcher.matchesAny() &&
                            !destinationInterfacesMatcher.matches(destinationFullPath.c_str()) &&
                            !destinationInterfacesMatcher.matches(destinationShortenedFullPath.c_str()))
                            continue;
                        InterfaceEntry *destinationInterfaceEntry = destinationInterfaceInfo->interfaceEntry;
                        Ipv4Address destinationAddress = destinationInterfaceInfo->getAddress();
                        Ipv4Address destinationNetmask = destinationInterfaceInfo->getNetmask();
                        if (!destinationInterfaceEntry->isLoopback() && !destinationAddress.isUnspecified()) {
                            Ipv4Route *route = new Ipv4Route();
                            Ipv4Address gatewayAddress = nextHopInterfaceInfo->getAddress();
                            if (addSubnetRoutesParameter && destinationNode->interfaceInfos.size() == 1 && destinationNode->interfaceInfos[0]->linkInfo->gatewayInterfaceInfo
                                && destinationNode->interfaceInfos[0]->addSubnetRoute)
                            {
                                ASSERT(!destinationAddress.doAnd(destinationNetmask).isUnspecified());
                                route->setDestination(destinationAddress.doAnd(destinationNetmask));
                                route->setNetmask(destinationNetmask);
                            }
                            else {
                                route->setDestination(destinationAddress);
                                route->setNetmask(Ipv4Address::ALLONES_ADDRESS);
                            }
                            route->setInterface(sourceInterfaceEntry);
                            if (gatewayAddress != destinationAddress)
                                route->setGateway(gatewayAddress);
                            route->setSourceType(Ipv4Route::MANUAL);
                            if (containsRoute(sourceNode->staticRoutes, route))
                                delete route;
                            else if (!addDirectRoutesParameter && route->getGateway().isUnspecified())
                                delete route;
                            else {
                                sourceNode->staticRoutes.push_back(route);
                                EV_DEBUG << "Adding route " << sourceInterfaceEntry->getInterfaceFullPath() << " -> " << destinationInterfaceEntry->getInterfaceFullPath() << " as " << route->str() << endl;
                            }
                        }
                    }
                }
            }

            // optimize routing table to save memory and increase lookup performance
            if (optimizeRoutesParameter)
                optimizeRoutes(sourceNode->staticRoutes);
        }
    }
}

/**
 * Returns true if the two routes are the same except their address prefix and netmask.
 * If it returns true we say that the routes have the same color.
 */
bool Ipv4NetworkConfigurator::routesHaveSameColor(Ipv4Route *route1, Ipv4Route *route2)
{
    return route1->getSourceType() == route2->getSourceType() && route1->getMetric() == route2->getMetric() &&
           route1->getGateway() == route2->getGateway() && route1->getInterface() == route2->getInterface();
}

/**
 * Returns the index of the first route that has the same color.
 */
int Ipv4NetworkConfigurator::findRouteIndexWithSameColor(const std::vector<Ipv4Route *>& routes, Ipv4Route *route)
{
    for (size_t i = 0; i < routes.size(); i++)
        if (routesHaveSameColor(routes[i], route))
            return i;

    return -1;
}

/**
 * Returns true if swapping two ADJACENT routes in the routing table does not change the table's meaning.
 */
bool Ipv4NetworkConfigurator::routesCanBeSwapped(RouteInfo *routeInfo1, RouteInfo *routeInfo2)
{
    if (routeInfo1->color == routeInfo2->color)
        return true; // these two routes send the packet in the same direction (same gw/iface), doesn't matter which one we use -> can be swapped
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
bool Ipv4NetworkConfigurator::routesCanBeNeighbors(const std::vector<RouteInfo *>& routeInfos, int i, int j)
{
    int begin = std::min(i, j);
    int end = std::max(i, j);
    Ipv4NetworkConfigurator::RouteInfo *beginRouteInfo = routeInfos.at(begin);
    for (int index = begin + 1; index < end; index++)
        if (!routesCanBeSwapped(beginRouteInfo, routeInfos.at(index)))
            return false;

    return true;
}

/**
 * Returns true if the original route is interrupted by any of the routes in
 * the routing table between begin and end.
 */
bool Ipv4NetworkConfigurator::interruptsOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, RouteInfo *originalRouteInfo)
{
    Ipv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination, begin, end);
    return matchingRouteInfo && matchingRouteInfo->color != originalRouteInfo->color;
}

/**
 * Returns true if any of the original routes is interrupted by any of the
 * routes in the routing table between begin and end.
 */
bool Ipv4NetworkConfigurator::interruptsAnyOriginalRoute(const RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos)
{
    if (begin < end)
        for (auto & originalRouteInfo : originalRouteInfos)
            if (interruptsOriginalRoute(routingTableInfo, begin, end, originalRouteInfo))
                return true;

    return false;
}

/**
 * Returns true if any of the original routes attached to the routes in the
 * routing table below index are interrupted by the route at index.
 */
bool Ipv4NetworkConfigurator::interruptsSubsequentOriginalRoutes(const RoutingTableInfo& routingTableInfo, int index)
{
    for (size_t i = index + 1; i < routingTableInfo.routeInfos.size(); i++) {
        Ipv4NetworkConfigurator::RouteInfo *routeInfo = routingTableInfo.routeInfos.at(i);
        if (interruptsAnyOriginalRoute(routingTableInfo, index, index + 1, routeInfo->originalRouteInfos))
            return true;
    }
    return false;
}

/**
 * Asserts that all original routes are still routed the same way as by the original routing table.
 */
void Ipv4NetworkConfigurator::checkOriginalRoutes(const RoutingTableInfo& routingTableInfo, const RoutingTableInfo& originalRoutingTableInfo)
{
    // assert that all original routes are routed with the same color
    for (auto & originalRouteInfo : originalRoutingTableInfo.routeInfos) {
        Ipv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination);
        Ipv4NetworkConfigurator::RouteInfo *matchingOriginalRouteInfo = originalRoutingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination);
        ASSERT(matchingRouteInfo && matchingRouteInfo->color == matchingOriginalRouteInfo->color);
    }
}

/**
 * Returns the longest shared address prefix and netmask by iterating through bits from left to right.
 */
void Ipv4NetworkConfigurator::findLongestCommonDestinationPrefix(uint32 destination1, uint32 netmask1, uint32 destination2, uint32 netmask2, uint32& destinationOut, uint32& netmaskOut)
{
    netmaskOut = 0;
    destinationOut = 0;
    for (int bitIndex = 31; bitIndex >= 0; bitIndex--) {
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
void Ipv4NetworkConfigurator::addOriginalRouteInfos(RoutingTableInfo& routingTableInfo, int begin, int end, const std::vector<RouteInfo *>& originalRouteInfos)
{
    for (auto & originalRouteInfo : originalRouteInfos) {
        Ipv4NetworkConfigurator::RouteInfo *matchingRouteInfo = routingTableInfo.findBestMatchingRouteInfo(originalRouteInfo->destination, begin, end);
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
bool Ipv4NetworkConfigurator::tryToMergeTwoRoutes(RoutingTableInfo& routingTableInfo, int i, int j, RouteInfo *routeInfoI, RouteInfo *routeInfoJ)
{
    // determine longest shared address prefix and netmask by iterating through bits from left to right
    uint32 netmask;
    uint32 destination;
    findLongestCommonDestinationPrefix(routeInfoI->destination, routeInfoI->netmask, routeInfoJ->destination, routeInfoJ->netmask, destination, netmask);

    // create the merged route
    ASSERT(routeInfoI->color == routeInfoJ->color);
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
    if (!interruptsAnyOriginalRoute(routingTableInfo, j + 1, i, routeInfoJ->originalRouteInfos) &&    // check that original routes on J are not interrupted between J and I
        !interruptsAnyOriginalRoute(routingTableInfo, i + 1, m, routeInfoJ->originalRouteInfos) &&    // check that original routes on J are not interrupted between I and M
        !interruptsAnyOriginalRoute(routingTableInfo, i + 1, m, routeInfoI->originalRouteInfos) &&    // check that original routes on I are not interrupted between I and M
        !interruptsSubsequentOriginalRoutes(routingTableInfo, m))    // check that the original routes after M are not interrupted by M
    {
        // now we know that the merge does not conflict with any route in the routing table.
        // the next thing to do is to maintain the original routes attached to the optimized ones.
        // move original routes from the to be deleted I route to the capturing routes.
        addOriginalRouteInfos(routingTableInfo, i + 1, m + 1, routeInfoI->originalRouteInfos);

        // move original routes from the to be deleted J route to the capturing routes.
        addOriginalRouteInfos(routingTableInfo, j + 1, m + 1, routeInfoJ->originalRouteInfos);

        // move original routes from the routes following the merged one if necessary.
        for (int k = m + 1; k < (int)routingTableInfo.routeInfos.size(); k++) {
            Ipv4NetworkConfigurator::RouteInfo *followingRouteInfo = routingTableInfo.routeInfos.at(k);
            for (int l = 0; l < (int)followingRouteInfo->originalRouteInfos.size(); l++) {
                Ipv4NetworkConfigurator::RouteInfo *originalRouteInfo = followingRouteInfo->originalRouteInfos.at(l);
                if (!((originalRouteInfo->destination ^ mergedRouteInfo->destination) & mergedRouteInfo->netmask)) {
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
        //checkOriginalRoutes(routingTableInfo, originalRoutingTableInfos);
#endif // ifndef NDEBUG
        delete routeInfoI;
        delete routeInfoJ;
        return true;
    }
    else {
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
bool Ipv4NetworkConfigurator::tryToMergeAnyTwoRoutes(RoutingTableInfo& routingTableInfo)
{
    for (int i = 0; i < (int)routingTableInfo.routeInfos.size(); i++) {
        Ipv4NetworkConfigurator::RouteInfo *routeInfoI = routingTableInfo.routeInfos.at(i);

        // iterate backward so that we try to merge routes having longer netmasks first.
        // this results in smaller changes and allows more symmetric optimization.
        for (int j = i - 1; j >= 0; j--) {
            Ipv4NetworkConfigurator::RouteInfo *routeInfoJ = routingTableInfo.routeInfos.at(j);

            // we can only merge neighbor routes having the same color
            if (routeInfoI->color == routeInfoJ->color && routesCanBeNeighbors(routingTableInfo.routeInfos, i, j)) {
                // it is worth to actually try to merge them
                if (tryToMergeTwoRoutes(routingTableInfo, i, j, routeInfoI, routeInfoJ))
                    return true;
            }
        }
    }
    return false;
}

void Ipv4NetworkConfigurator::optimizeRoutes(std::vector<Ipv4Route *>& originalRoutes)
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
    // instead of working with Ipv4 routes we transform them into the internal representation of the optimizer.
    // routes are classified based on their action (gateway, interface, type, source, metric, etc.) and a color is assigned to them.
    RoutingTableInfo routingTableInfo;
    std::vector<Ipv4Route *> colorToRoute;    // a mapping from color to route action (interface, gateway, metric, etc.)
    RoutingTableInfo originalRoutingTableInfo;    // a copy of the original routes in the optimizer's format

    // build colorToRouteColor, originalRoutingTableInfo and initial routeInfos in routingTableInfo
    for (auto & originalRoute : originalRoutes) {
        int color = findRouteIndexWithSameColor(colorToRoute, originalRoute);
        if (color == -1) {
            color = colorToRoute.size();
            colorToRoute.push_back(originalRoute);
        }

        // create original route and determine its color
        RouteInfo *originalRouteInfo = new RouteInfo(color, originalRoute->getDestination().getInt(), originalRoute->getNetmask().getInt());
        originalRoutingTableInfo.addRouteInfo(originalRouteInfo);

        // create a copy of the original route that can be destructively optimized later
        RouteInfo *optimizedRouteInfo = new RouteInfo(*originalRouteInfo);
        optimizedRouteInfo->originalRouteInfos.push_back(originalRouteInfo);
        routingTableInfo.addRouteInfo(optimizedRouteInfo);
    }

#ifndef NDEBUG
    checkOriginalRoutes(routingTableInfo, originalRoutingTableInfo);
#endif // ifndef NDEBUG

    // STEP 2.
    // from now on we are only working with the internal data structures called RouteInfo and RoutingTableInfo.
    // the main optimizer loop runs until it cannot merge any two routes.
    while (tryToMergeAnyTwoRoutes(routingTableInfo))
        ;

#ifndef NDEBUG
    checkOriginalRoutes(routingTableInfo, originalRoutingTableInfo);
#endif // ifndef NDEBUG

    for (auto rti: originalRoutingTableInfo.routeInfos)
        delete rti;
    originalRoutingTableInfo.routeInfos.clear();

    // STEP 3.
    // convert the optimized routes to new optimized Ipv4 routes based on the saved colors
    std::vector<Ipv4Route *> optimizedRoutes;
    for (auto & routeInfo : routingTableInfo.routeInfos) {
        Ipv4Route *routeColor = colorToRoute[routeInfo->color];
        Ipv4Route *optimizedRoute = new Ipv4Route();
        optimizedRoute->setDestination(Ipv4Address(routeInfo->destination));
        optimizedRoute->setNetmask(Ipv4Address(routeInfo->netmask));
        optimizedRoute->setInterface(routeColor->getInterface());
        optimizedRoute->setGateway(routeColor->getGateway());
        optimizedRoute->setSourceType(routeColor->getSourceType());
        optimizedRoute->setMetric(routeColor->getMetric());
        optimizedRoutes.push_back(optimizedRoute);
        delete routeInfo;
    }

    // delete original routes, we destructively modify them
    for (auto & originalRoute : originalRoutes)
        delete originalRoute;

    // copy optimized routes to original routes and return
    originalRoutes = optimizedRoutes;
}

bool Ipv4NetworkConfigurator::getInterfaceIpv4Address(L3Address& ret, InterfaceEntry *interfaceEntry, bool netmask)
{
    auto it = topology.interfaceInfos.find(interfaceEntry->getId());
    if (it == topology.interfaceInfos.end())
        return false;
    else {
        InterfaceInfo *interfaceInfo = static_cast<InterfaceInfo *>(it->second);
        if (interfaceInfo->configure)
            ret = netmask ? interfaceInfo->getNetmask() : interfaceInfo->getAddress();
        return interfaceInfo->configure;
    }
}

} // namespace inet

