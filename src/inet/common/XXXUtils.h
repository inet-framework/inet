//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_NETWORKUTILS_H_
#define __INET_NETWORKUTILS_H_

#include <functional>
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

/**
 * This file provides several utility functions for navigating between the
 * following objects in the network:
 *  - modules
 *  - the network
 *  - subnetworks
 *  - network nodes
 *  - network interfaces
 *  - routing tables
 *  - interface tables
 *  - IPv4 addresses
 *  - IPv6 addresses
 *  - MAC addresses
 */
namespace inet {

// network -> modules
void mapModules(const std::function<void(cModule&)>& procedure);
void mapModules(const cSimulation& simulation, const std::function<void(cModule&)>& procedure);
void mapModulesRecursively(cModule& module, const std::function<void(cModule&)>& procedure);
void mapSubmodulesRecursively(const cModule& module, const std::function<void(cModule&)>& procedure);
// network -> network node
void mapNetworkNodes(const std::function<void(cModule&)>& procedure);
void mapNetworkNodes(const cSimulation& simulation, const std::function<void(cModule&)>& procedure);
void mapNetworkNodesRecursively(const cModule& module, const std::function<void(cModule&)>& procedure);
// network -> network interface
void mapNetworkInterfaces(const std::function<void(InterfaceEntry&)>& procedure);
void mapNetworkInterfaces(const cSimulation& simulation, const std::function<void(InterfaceEntry&)>& procedure);
void mapNetworkInterfaces(const cModule& module, const std::function<void(InterfaceEntry&)>& procedure);
// network -> IPv4 routing table
void mapIPv4RoutingTables(const std::function<void(IIPv4RoutingTable&)>& procedure);
void mapIPv4RoutingTables(const cSimulation& simulation, const std::function<void(IIPv4RoutingTable&)>& procedure);
void mapIPv4RoutingTables(const cModule& module, const std::function<void(IIPv4RoutingTable&)>& procedure);
// network -> IPv6 routing table
void mapIPv6RoutingTables(const std::function<void(IPv6RoutingTable&)>& procedure);
void mapIPv6RoutingTables(const cSimulation& simulation, const std::function<void(IPv6RoutingTable&)>& procedure);
void mapIPv6RoutingTables(const cModule& module, const std::function<void(IPv6RoutingTable&)>& procedure);
// network -> interface table
void mapInterfaceTables(const std::function<void(IInterfaceTable&)>& procedure);
void mapInterfaceTables(const cSimulation& simulation, const std::function<void(IInterfaceTable&)>& procedure);
void mapInterfaceTables(const cModule& module, const std::function<void(IInterfaceTable&)>& procedure);
// network -> IPv4 address
void mapIPv4Addresses(const std::function<void(IPv4Address&)>& procedure);
void mapIPv4Addresses(const cSimulation& simulation, const std::function<void(IPv4Address&)>& procedure);
void mapIPv4Addresses(const cModule& module, const std::function<void(IPv4Address&)>& procedure);
// network -> IPv6 address
void mapIPv6Addresses(const std::function<void(IPv6Address&)>& procedure);
void mapIPv6Addresses(const cSimulation& simulation, const std::function<void(IPv6Address&)>& procedure);
void mapIPv6Addresses(const cModule& module, const std::function<void(IPv6Address&)>& procedure);
// network -> MAC address
void mapMACAddresses(const std::function<void(MACAddress&)>& procedure);
void mapMACAddresses(const cSimulation& simulation, const std::function<void(MACAddress&)>& procedure);
void mapMACAddresses(const cModule& module, const std::function<void(MACAddress&)>& procedure);


// network node -> network interface
InterfaceEntry *findDefaultNetworkInterface(const cModule& networkNode);
InterfaceEntry& getDefaultNetworkInterface(const cModule& networkNode);
// network node -> IPv4 routing table
IIPv4RoutingTable *findDefaultIPv4RoutingTable(const cModule& networkNode);
IIPv4RoutingTable& getDefaultIPv4RoutingTable(const cModule& networkNode);
// network node -> IPv6 routing table
IPv6RoutingTable *findDefaultIPv6RoutingTable(const cModule& networkNode);
IPv6RoutingTable& getDefaultIPv6RoutingTable(const cModule& networkNode);
// network node -> interface table
IInterfaceTable *findDefaultInterfaceTable(const cModule& networkNode);
IInterfaceTable *getDefaultInterfaceTable(const cModule& networkNode);
// network node -> IPv4 address
IPv4Address findDefaultIPv4Address(const cModule& networkNode);
IPv4Address getDefaultIPv4Address(const cModule& networkNode);
// network node -> IPv6 address
IPv6Address findDefaultIPv6Address(const cModule& networkNode);
IPv6Address getDefaultIPv6Address(const cModule& networkNode);
// network node -> MAC address
MACAddress findDefaultMACAddress(const cModule& networkNode);
MACAddress getDefaultMACAddress(const cModule& networkNode);


// network interface -> network node
cModule& getNetworkNode(const InterfaceEntry& networkInterface);
// network interface -> IPv6 address
void mapIPv6Addresses(const InterfaceEntry& networkInterface, const std::function<void(IPv4Address&)>& procedure);


// IPv4 routing table -> network node
cModule& getNetworkNode(const IIPv4RoutingTable& routingTable);


// IPv6 routing table -> network node
cModule& getNetworkNode(const IPv6RoutingTable& routingTable);


// interface table -> network node
cModule& getNetworkNode(const IInterfaceTable& interfaceTable);
// interface table -> network interface
InterfaceEntry *findDefaultNetworkInterface(const IInterfaceTable& interfaceTable);
InterfaceEntry& getDefaultNetworkInterface(const IInterfaceTable& interfaceTable);
void mapNetworkInterfaces(const IInterfaceTable& interfaceTable, const std::function<void(InterfaceEntry&)>& procedure);
// interface table -> IPv4 address
IPv4Address findDefaultIPv4Address(const IInterfaceTable& interfaceTable);
IPv4Address getDefaultIPv4Address(const IInterfaceTable& interfaceTable);
void mapIPv4Addresses(const IInterfaceTable& interfaceTable, const std::function<void(IPv4Address&)>& procedure);
// interface table -> IPv6 address
IPv6Address findDefaultIPv6Address(const IInterfaceTable& interfaceTable);
IPv6Address getDefaultIPv6Address(const IInterfaceTable& interfaceTable);
void mapIPv6Addresses(const IInterfaceTable& interfaceTable, const std::function<void(IPv6Address&)>& procedure);
// interface table -> MAC address
MACAddress findDefaultMACAddress(const IInterfaceTable& interfaceTable);
MACAddress getDefaultMACAddress(const IInterfaceTable& interfaceTable);
void mapMACAddresses(const IInterfaceTable& interfaceTable, const std::function<void(MACAddress&)>& procedure);


// IPv4 address -> network node
cModule *findSingleNetworkNode(const IPv4Address& address);
cModule& getSingleNetworkNode(const IPv4Address& address);
void mapNetworkNodes(const IPv4Address& address, const std::function<void(cModule&)>& procedure);
// IPv4 address -> network interface
InterfaceEntry *findSingleNetworkInterface(const IPv4Address& address);
InterfaceEntry& getSingleNetworkInterface(const IPv4Address& address);
void mapNetworkInterfaces(const IPv4Address& address, const std::function<void(InterfaceEntry&)>& procedure);


// IPv6 address -> network node
cModule *findSingleNetworkNode(const IPv6Address& address);
cModule& getSingleNetworkNode(const IPv6Address& address);
void mapNetworkNodes(const IPv6Address& address, const std::function<void(cModule&)>& procedure);
// IPv6 address -> network interface
InterfaceEntry *findSingleNetworkInterface(const IPv6Address& address);
InterfaceEntry& getSingleNetworkInterface(const IPv6Address& address);
void mapNetworkInterfaces(const IPv6Address& address, const std::function<void(InterfaceEntry&)>& procedure);


// MAC address -> network node
cModule *findSingleNetworkNode(const MACAddress& address);
cModule& getSingleNetworkNode(const MACAddress& address);
void mapNetworkNodes(const MACAddress& address, const std::function<void(cModule&)>& procedure);
// MAC address -> network interface
InterfaceEntry *findSingleNetworkInterface(const MACAddress& address);
InterfaceEntry& getSingleNetworkInterface(const MACAddress& address);
void mapNetworkInterfaces(const MACAddress& address, const std::function<void(InterfaceEntry&)>& procedure);


} // namespace

#endif // #ifndef __INET_NETWORKUTILS_H_

