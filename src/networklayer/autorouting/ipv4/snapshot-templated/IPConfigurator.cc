//
// Copyright (C) 2011 Opensim Ltd
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

#include "stlutils.h"
#include "IPConfigurator.h"
#include "IPvXAddressResolver.h"

static Topology::LinkOut *findLinkOut(Topology::Node *node, int gateId)
{
    for (int i = 0; i < node->getNumOutLinks(); i++)
        if (node->getLinkOut(i)->getLocalGateId() == gateId)
            return node->getLinkOut(i);
    return NULL;
}

template <typename IPUINT>
void IPConfigurator<IPUINT>::extractTopology(Topology& topology, NetworkInfo& networkInfo)
{
    // extract topology
    topology.extractByProperty("node");
    EV_DEBUG << "Topology found " << topology.getNumNodes() << " nodes\n";

    // extract nodes, fill in isIPNode, interfaceTable and routingTable members in nodeInfo[]
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Topology::Node *node = topology.getNode(i);
        cModule *module = node->getModule();
        NodeInfo *nodeInfo = new NodeInfo(module);
        node->setPayload(nodeInfo);
        nodeInfo->module = module;
        nodeInfo->interfaceTable = IPvXAddressResolver().findInterfaceTableOf(module);
        nodeInfo->routingTable = IPvXAddressResolver().findRoutingTableOf(module);
        nodeInfo->isIPNode = nodeInfo->interfaceTable != NULL;
        if (nodeInfo->isIPNode && nodeInfo->routingTable && !nodeInfo->routingTable->isIPForwardingEnabled())
            node->setWeight(DBL_MAX);
    }

    // extract links and interfaces
    std::set<InterfaceEntry*> interfacesSeen;
    for (int i = 0; i < topology.getNumNodes(); i++) {
        Topology::Node *node = topology.getNode(i);
        NodeInfo *nodeInfo = (NodeInfo *)node->getPayload();
        cModule *module = node->getModule();
        IInterfaceTable *interfaceTable = IPvXAddressResolver().findInterfaceTableOf(module);
        if (interfaceTable) {
            for (int j = 0; j < interfaceTable->getNumInterfaces(); j++) {
                InterfaceEntry *interfaceEntry = interfaceTable->getInterface(j);
                if (!interfaceEntry->isLoopback() && interfacesSeen.count(interfaceEntry) == 0) {
                    // store interface as belonging to a new network link
                    LinkInfo *linkInfo = new LinkInfo();
                    networkInfo.linkInfos.push_back(linkInfo);
                    linkInfo->interfaceInfos.push_back(createInterfaceInfo(nodeInfo, linkInfo, interfaceEntry));
                    interfacesSeen.insert(interfaceEntry);

                    // visit neighbor (and potentially the whole LAN, recursively)
                    Topology::LinkOut *linkOut = findLinkOut(topology.getNode(i), interfaceEntry->getNodeOutputGateId());
                    if (linkOut) {
                        std::vector<Topology::Node*> empty;
                        extractNeighbors(linkOut, linkInfo, interfacesSeen, empty);
                    }

                    // determine gatewayInterfaceInfo
                    for (int k = 0; k < linkInfo->interfaceInfos.size(); k++) {
                        InterfaceInfo *interfaceInfo = linkInfo->interfaceInfos[k];
                        IInterfaceTable *interfaceTable = interfaceInfo->nodeInfo->interfaceTable;
                        // count non-loopback source interfaces
                        int nonLoopbackInterfaceCount = 0;
                        for (int l = 0; l < interfaceTable->getNumInterfaces(); l++)
                            if (!interfaceTable->getInterface(l)->isLoopback())
                                nonLoopbackInterfaceCount++;
                        // check for having a single gateway interface
                        if (nonLoopbackInterfaceCount > 1) {
                            if (linkInfo->gatewayInterfaceInfo) {
                                linkInfo->gatewayInterfaceInfo = NULL;
                                break;
                            }
                            else
                                linkInfo->gatewayInterfaceInfo = interfaceInfo;
                        }
                    }
                }
            }
        }
    }
}

template <typename IPUINT>
void IPConfigurator<IPUINT>::extractNeighbors(Topology::LinkOut *linkOut, LinkInfo* linkInfo, std::set<InterfaceEntry*>& interfacesSeen, std::vector<Topology::Node*>& deviceNodesVisited)
{
    Topology::Node *neighborNode = linkOut->getRemoteNode();
    cModule *neighborModule = neighborNode->getModule();
    NodeInfo *neighborNodeInfo = (NodeInfo *)neighborNode->getPayload();
    int neighborInputGateId = linkOut->getRemoteGateId();
    IInterfaceTable *neighborInterfaceTable = IPvXAddressResolver().findInterfaceTableOf(neighborModule);
    if (neighborInterfaceTable) {
        // neighbor is a host or router, just add the interface
        InterfaceEntry *neighborInterfaceEntry = neighborInterfaceTable->getInterfaceByNodeInputGateId(neighborInputGateId);
        if (interfacesSeen.count(neighborInterfaceEntry) == 0) {
            linkInfo->interfaceInfos.push_back(createInterfaceInfo(neighborNodeInfo, linkInfo, neighborInterfaceEntry));
            interfacesSeen.insert(neighborInterfaceEntry);
        }
    }
    else {
        // assume that neighbor is an L2 or L1 device (bus/hub/switch/bridge/access point/etc); visit all its output links
        Topology::Node *deviceNode = linkOut->getRemoteNode();
        if (!contains(deviceNodesVisited, deviceNode)) {
            deviceNodesVisited.push_back(deviceNode);
            for (int i = 0; i < deviceNode->getNumOutLinks(); i++) {
                Topology::LinkOut *deviceLinkOut = deviceNode->getLinkOut(i);
                extractNeighbors(deviceLinkOut, linkInfo, interfacesSeen, deviceNodesVisited);
            }
        }
    }
}

template <typename IPUINT>
typename IPConfigurator<IPUINT>::InterfaceInfo *IPConfigurator<IPUINT>::createInterfaceInfo(NodeInfo *nodeInfo, LinkInfo* linkInfo, InterfaceEntry *interfaceEntry)
{
    InterfaceInfo *interfaceInfo = new InterfaceInfo(nodeInfo, linkInfo, interfaceEntry);
    nodeInfo->interfaceInfos.push_back(interfaceInfo);
    return interfaceInfo;
}

/**
 * Returns how many bits are needed to represent count different values.
 */
template <typename IPUINT>
inline int getRepresentationBitCount(IPUINT count)
{
    int bitCount = 0;
    while (((IPUINT)1 << bitCount) < count)
        bitCount++;
    return bitCount;
}

/**
 * Returns the index of the most significant bit that equals to the given bit value.
 * 0 means the most significant bit.
 */
template <typename IPUINT>
static int getMostSignificantBitIndex(IPUINT value, int bitValue, int defaultIndex)
{
    for (int bitIndex = sizeof(value) * 8 - 1; bitIndex >= 0; bitIndex--) {
        IPUINT mask = (IPUINT)1 << bitIndex;
        if ((value & mask) == ((IPUINT)bitValue << bitIndex))
            return bitIndex;
    }
    return defaultIndex;
}

/**
 * Returns the index of the least significant bit that equals to the given bit value.
 * 0 means the most significant bit.
 */
template <typename IPUINT>
static int getLeastSignificantBitIndex(IPUINT value, int bitValue, int defaultIndex)
{
    for (int bitIndex = 0; bitIndex < sizeof(value) * 8; bitIndex++) {
        IPUINT mask = (IPUINT)1 << bitIndex;
        if ((value & mask) == ((IPUINT)bitValue << bitIndex))
            return bitIndex;
    }
    return defaultIndex;
}

/**
 * Returns packed bits (subsequent) from value specified by mask (sparse).
 */
template <typename IPUINT>
static IPUINT getPackedBits(IPUINT value, IPUINT valueMask)
{
    IPUINT packedValue = 0;
    int packedValueIndex = 0;
    for (int valueIndex = 0; valueIndex < sizeof(value) * 8; valueIndex++) {
        IPUINT valueBitMask = (IPUINT)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0) {
            if ((value & valueBitMask) != 0)
                packedValue |= (IPUINT)1 << packedValueIndex;
            packedValueIndex++;
        }
    }
    return packedValue;
}

/**
 * Set packed bits (subsequent) in value specified by mask (sparse).
 */
template <typename IPUINT>
static IPUINT setPackedBits(IPUINT value, IPUINT valueMask, IPUINT packedValue)
{
    int packedValueIndex = 0;
    for (int valueIndex = 0; valueIndex < sizeof(value) * 8; valueIndex++) {
        IPUINT valueBitMask = (IPUINT)1 << valueIndex;
        if ((valueMask & valueBitMask) != 0) {
            IPUINT newValueBitMask = (IPUINT)1 << packedValueIndex;
            if ((packedValue & newValueBitMask) != 0)
                value |= valueBitMask;
            else
                value &= ~valueBitMask;
            packedValueIndex++;
        }
    }
    return value;
}

template <typename IPUINT>
void IPConfigurator<IPUINT>::assignAddresses(Topology& topology, NetworkInfo& networkInfo)
{
    int bitSize = sizeof(IPUINT) * 8;
    std::vector<IPUINT> assignedNetworkAddresses;
    std::vector<IPUINT> assignedNetworkNetmasks;
    std::vector<IPUINT> assignedInterfaceAddresses;
    std::map<IPUINT, InterfaceEntry *> assignedAddressToInterfaceEntryMap;
    // iterate through all links and process them separately one by one
    for (int linkIndex = 0; linkIndex < networkInfo.linkInfos.size(); linkIndex++) {
        LinkInfo *selectedLink = networkInfo.linkInfos[linkIndex];
        // repeat until all interfaces of the selected link become configured
        // and assign addresses to groups of interfaces having compatible address and netmask specifications
        std::vector<InterfaceInfo*> unconfiguredInterfaces = selectedLink->interfaceInfos;
        while (unconfiguredInterfaces.size() != 0) {

            // STEP 1.
            // find a subset of the unconfigured interfaces that have compatible address and netmask specifications
            // determine the merged address and netmask specifications according to the following table
            // the '?' symbol means the bit is unspecified, the 'X' symbol means the bit is incompatible
            // | * | 0 | 1 | ? |
            // | 0 | 0 | X | 0 |
            // | 1 | X | 1 | 1 |
            // | ? | 0 | 1 | ? |
            // the result of step 1 is the following:
            IPUINT mergedAddress = 0;                 // compatible bits of the merged address (both 0 and 1 are address bits)
            IPUINT mergedAddressSpecifiedBits = 0;    // mask for the valid compatible bits of the merged address (0 means unspecified, 1 means specified)
            IPUINT mergedAddressIncompatibleBits = 0; // incompatible bits of the merged address (0 means compatible, 1 means incompatible)
            IPUINT mergedNetmask = 0;                 // compatible bits of the merged netmask (both 0 and 1 are netmask bits)
            IPUINT mergedNetmaskSpecifiedBits = 0;    // mask for the compatible bits of the merged netmask (0 means unspecified, 1 means specified)
            IPUINT mergedNetmaskIncompatibleBits = 0; // incompatible bits of the merged netmask (0 means compatible, 1 means incompatible)
            std::vector<InterfaceInfo*> compatibleInterfaces; // the list of compatible interfaces
            for (int unconfiguredInterfaceIndex = 0; unconfiguredInterfaceIndex < unconfiguredInterfaces.size(); unconfiguredInterfaceIndex++) {
                InterfaceInfo *candidateInterface = unconfiguredInterfaces[unconfiguredInterfaceIndex];
                InterfaceEntry *interfaceEntry = candidateInterface->interfaceEntry;
                // extract candidate interface configuration data   //FIXME az ilyen kommentek ele ures sort
                IPUINT candidateAddress = candidateInterface->address;
                IPUINT candidateAddressSpecifiedBits = candidateInterface->addressSpecifiedBits;
                IPUINT candidateNetmask = candidateInterface->netmask;
                IPUINT candidateNetmaskSpecifiedBits = candidateInterface->netmaskSpecifiedBits;
                EV_DEBUG << "Trying to merge " << interfaceEntry->getFullPath() << " interface with address specification: " << toString(candidateAddress) << " / " << toString(candidateAddressSpecifiedBits) << endl;
                EV_DEBUG << "Trying to merge " << interfaceEntry->getFullPath() << " interface with netmask specification: " << toString(candidateNetmask) << " / " << toString(candidateNetmaskSpecifiedBits) << endl;
                // determine merged netmask bits
                IPUINT commonNetmaskSpecifiedBits = mergedNetmaskSpecifiedBits & candidateNetmaskSpecifiedBits;
                IPUINT newMergedNetmask = mergedNetmask | (candidateNetmask & candidateNetmaskSpecifiedBits);
                IPUINT newMergedNetmaskSpecifiedBits = mergedNetmaskSpecifiedBits | candidateNetmaskSpecifiedBits;
                IPUINT newMergedNetmaskIncompatibleBits = mergedNetmaskIncompatibleBits | ((mergedNetmask & commonNetmaskSpecifiedBits) ^ (candidateNetmask & commonNetmaskSpecifiedBits));
                // skip interface if there's a bit where the netmasks are incompatible
                if (newMergedNetmaskIncompatibleBits != 0)
                    continue;
                // determine merged address bits
                IPUINT commonAddressSpecifiedBits = mergedAddressSpecifiedBits & candidateAddressSpecifiedBits;
                IPUINT newMergedAddress = mergedAddress | (candidateAddress & candidateAddressSpecifiedBits);
                IPUINT newMergedAddressSpecifiedBits = mergedAddressSpecifiedBits | candidateAddressSpecifiedBits;
                IPUINT newMergedAddressIncompatibleBits = mergedAddressIncompatibleBits | ((mergedAddress & commonAddressSpecifiedBits) ^ (candidateAddress & commonAddressSpecifiedBits));
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
                EV_DEBUG << "Merged address specification: " << toString(mergedAddress) << " / " << toString(mergedAddressSpecifiedBits) << " / " << toString(mergedAddressIncompatibleBits) << endl;
                EV_DEBUG << "Merged netmask specification: " << toString(mergedNetmask) << " / " << toString(mergedNetmaskSpecifiedBits) << " / " << toString(mergedNetmaskIncompatibleBits) << endl;
            }
            EV_DEBUG << "Found " << compatibleInterfaces.size() << " compatible interfaces" << endl;

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
            IPUINT networkAddress = 0; // network part of the addresses  (e.g. 10.1.1.0)
            IPUINT networkNetmask = 0; // netmask for the network (e.g. 255.255.255.0)
            for (netmaskLength = maximumNetmaskLength; netmaskLength >= minimumNetmaskLength; netmaskLength--) {
                networkNetmask = (((IPUINT)1 << netmaskLength) - (IPUINT)1) << (bitSize - netmaskLength);
                EV_DEBUG << "Trying network netmask: " << toString(networkNetmask) << " : " << netmaskLength << endl;
                networkAddress = mergedAddress & mergedAddressSpecifiedBits & networkNetmask;
                IPUINT networkAddressUnspecifiedBits = ~mergedAddressSpecifiedBits & networkNetmask; // 1 means the network address unspecified
                IPUINT networkAddressUnspecifiedPartMaximum = 0;
                for (int i = 0; i < assignedNetworkAddresses.size(); i++) {
                    IPUINT assignedNetworkAddress = assignedNetworkAddresses[i];
                    IPUINT assignedNetworkNetmask = assignedNetworkNetmasks[i];
                    IPUINT assignedNetworkAddressMaximum = assignedNetworkAddress | ~assignedNetworkNetmask;
                    EV_DEBUG << "Checking against assigned network address " << toString(assignedNetworkAddress) << endl;
                    if ((assignedNetworkAddress & ~networkAddressUnspecifiedBits) == (networkAddress & ~networkAddressUnspecifiedBits)) {
                        IPUINT assignedAddressUnspecifiedPart = getPackedBits(assignedNetworkAddressMaximum, networkAddressUnspecifiedBits);
                        if (assignedAddressUnspecifiedPart > networkAddressUnspecifiedPartMaximum)
                            networkAddressUnspecifiedPartMaximum = assignedAddressUnspecifiedPart;
                    }
                }
                IPUINT networkAddressUnspecifiedPartLimit = getPackedBits(~(IPUINT)0, networkAddressUnspecifiedBits) + (IPUINT)1;
                EV_DEBUG << "Counting from: " << networkAddressUnspecifiedPartMaximum + (IPUINT)1 << " to: " << networkAddressUnspecifiedPartLimit << endl;
                // we start with +1 so that the network address will be more likely different
                for (IPUINT networkAddressUnspecifiedPart = networkAddressUnspecifiedPartMaximum; networkAddressUnspecifiedPart <= networkAddressUnspecifiedPartLimit; networkAddressUnspecifiedPart++) {
                    networkAddress = setPackedBits(networkAddress, networkAddressUnspecifiedBits, networkAddressUnspecifiedPart);
                    EV_DEBUG << "Trying network address: " << toString(networkAddress) << endl;
                    // count interfaces that have the same address prefix
                    int interfaceCount = 0;
                    for (int i = 0; i < assignedInterfaceAddresses.size(); i++)
                        if ((assignedInterfaceAddresses[i] & networkNetmask) == networkAddress)
                            interfaceCount++;
                    if (interfaceCount != 0 && par("disjunctSubnetAddresses").boolValue())
                        continue;
                    EV_DEBUG << "Matching interface count: " << interfaceCount << endl;
                    // check if there's enough room for the interface addresses
                    if ((1 << (bitSize - netmaskLength)) >= interfaceCount + compatibleInterfaceCount)
                        goto found;
                }
            }
            found: if (netmaskLength < minimumNetmaskLength || netmaskLength > maximumNetmaskLength)
                throw cRuntimeError("Failed to configure address prefix and netmask for %s and %d other interface(s). Please refine your parameters and try again!",
                    compatibleInterfaces[0]->interfaceEntry->getFullPath().c_str(), compatibleInterfaces.size() - 1);
            EV_DEBUG << "Selected netmask length: " << netmaskLength << endl;
            EV_DEBUG << "Selected network address: " << toString(networkAddress) << endl;
            EV_DEBUG << "Selected network netmask: " << toString(networkNetmask) << endl;

            // STEP 4.
            // determine the complete IP address for all compatible interfaces
            for (int interfaceIndex = 0; interfaceIndex < compatibleInterfaces.size(); interfaceIndex++) {
                InterfaceInfo *compatibleInterface = compatibleInterfaces[interfaceIndex];
                InterfaceEntry *interfaceEntry = compatibleInterface->interfaceEntry;
                IPUINT interfaceAddress = compatibleInterface->address & ~networkNetmask;
                IPUINT interfaceAddressSpecifiedBits = compatibleInterface->addressSpecifiedBits;
                IPUINT interfaceAddressUnspecifiedBits = ~interfaceAddressSpecifiedBits & ~networkNetmask; // 1 means the interface address is unspecified
                IPUINT interfaceAddressUnspecifiedPartMaximum = 0;
                for (int i = 0; i < assignedInterfaceAddresses.size(); i++) {
                    IPUINT otherInterfaceAddress = assignedInterfaceAddresses[i];
                    if ((otherInterfaceAddress & ~interfaceAddressUnspecifiedBits) == ((networkAddress | interfaceAddress) & ~interfaceAddressUnspecifiedBits)) {
                        IPUINT otherInterfaceAddressUnspecifiedPart = getPackedBits(otherInterfaceAddress, interfaceAddressUnspecifiedBits);
                        if (otherInterfaceAddressUnspecifiedPart > interfaceAddressUnspecifiedPartMaximum)
                            interfaceAddressUnspecifiedPartMaximum = otherInterfaceAddressUnspecifiedPart;
                    }
                }
                interfaceAddressUnspecifiedPartMaximum++;
                interfaceAddress = setPackedBits(interfaceAddress, interfaceAddressUnspecifiedBits, interfaceAddressUnspecifiedPartMaximum);
                // determine the complete address and netmask for interface
                IPUINT completeAddress = networkAddress | interfaceAddress;
                IPUINT completeNetmask = networkNetmask;
                // check if we could really find a unique IP address
                if (assignedAddressToInterfaceEntryMap.find(completeAddress) != assignedAddressToInterfaceEntryMap.end())
                    cRuntimeError("Failed to configure address and netmask for %s. Please refine your parameters and try again!", interfaceEntry->getFullPath().c_str());
                assignedAddressToInterfaceEntryMap[completeAddress] = compatibleInterface->interfaceEntry;
                assignedInterfaceAddresses.push_back(completeAddress);
                // configure interface with the selected address and netmask
                if (compatibleInterface->configure)
                    assignAddress(compatibleInterface->interfaceEntry, completeAddress, completeNetmask);
                compatibleInterface->address = completeAddress;
                EV_DEBUG << "Selected interface address: " << toString(completeAddress) << endl;
                // remove configured interface
                unconfiguredInterfaces.erase(find(unconfiguredInterfaces, compatibleInterface));
            }
            // register the network address and netmask as being used
            assignedNetworkAddresses.push_back(networkAddress);
            assignedNetworkNetmasks.push_back(networkNetmask);
        }
    }
}

template <typename IPUINT>
void IPConfigurator<IPUINT>::dumpAddresses(NetworkInfo& networkInfo)
{
    for (int i = 0; i < networkInfo.linkInfos.size(); i++) {
        EV_INFO << "Link " << i << endl;
        LinkInfo* linkInfo = networkInfo.linkInfos[i];
        for (int j = 0; j < linkInfo->interfaceInfos.size(); j++) {
            InterfaceEntry *interfaceEntry = linkInfo->interfaceInfos[j]->interfaceEntry;
            cModule *host = dynamic_cast<cModule *>(interfaceEntry->getInterfaceTable())->getParentModule();
            EV_INFO << "    " << host->getFullName() << " / " << interfaceEntry->getName() << " " << interfaceEntry->info() << endl;
        }
    }
}
