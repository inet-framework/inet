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

#include "inet/common/ModuleAccess.h"
#include "inet/common/XXXUtils.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"

namespace inet {

// network -> modules
void mapModules(const std::function<void(cModule&)>& procedure)
{
    mapModules(*cSimulation::getActiveSimulation(), procedure);
}

void mapModules(const cSimulation& simulation, const std::function<void(cModule&)>& procedure)
{
    mapSubmodulesRecursively(*simulation.getSystemModule(), procedure);
}

void mapModulesRecursively(cModule& module, const std::function<void(cModule&)>& procedure)
{
    procedure(module);
    for (cModule::SubmoduleIterator it(&module); !it.end(); it++)
        mapModulesRecursively(**it, procedure);
}

void mapSubmodulesRecursively(const cModule& module, const std::function<void(cModule&)>& procedure)
{
    for (cModule::SubmoduleIterator it(&module); !it.end(); it++) {
        auto& submodule = **it;
        procedure(submodule);
        mapSubmodulesRecursively(submodule, procedure);
    }
}

// network -> network node
void mapNetworkNodes(const std::function<void(cModule&)>& procedure)
{
    mapNetworkNodes(*cSimulation::getActiveSimulation(), procedure);
}

void mapNetworkNodes(const cSimulation& simulation, const std::function<void(cModule&)>& procedure)
{
    mapNetworkNodesRecursively(*simulation.getSystemModule(), procedure);
}

void mapNetworkNodesRecursively(const cModule& module, const std::function<void(cModule&)>& procedure)
{
    for (cModule::SubmoduleIterator it(&module); !it.end(); it++) {
        auto& networkNode = **it;
        if (isNetworkNode(&networkNode))
            procedure(networkNode);
        else
            mapNetworkNodesRecursively(**it, procedure);
    }
}

// network -> network interface
void mapNetworkInterfaces(const std::function<void(InterfaceEntry&)>& procedure)
{
    mapNetworkInterfaces(*cSimulation::getActiveSimulation(), procedure);
}

void mapNetworkInterfaces(const cSimulation& simulation, const std::function<void(InterfaceEntry&)>& procedure)
{
    mapNetworkInterfaces(*simulation.getSystemModule(), procedure);
}

void mapNetworkInterfaces(const cModule& module, const std::function<void(InterfaceEntry&)>& procedure)
{
    mapInterfaceTables(module, [&] (IInterfaceTable& interfaceTable) {
        mapNetworkInterfaces(interfaceTable, procedure);
    });
}

// network -> IPv4 routing table
void mapIPv4RoutingTables(const std::function<void(IIPv4RoutingTable&)>& procedure)
{
    mapIPv4RoutingTables(*cSimulation::getActiveSimulation(), procedure);
}

void mapIPv4RoutingTables(const cSimulation& simulation, const std::function<void(IIPv4RoutingTable&)>& procedure)
{
    mapIPv4RoutingTables(*simulation.getSystemModule(), procedure);
}

void mapIPv4RoutingTables(const cModule& module, const std::function<void(IIPv4RoutingTable&)>& procedure)
{
    mapSubmodulesRecursively(module, [&] (cModule& module) {
        if (auto ipv4RoutingTable = dynamic_cast<IIPv4RoutingTable *>(&module))
            procedure(*ipv4RoutingTable);
    });
}

// network -> IPv6 routing table
void mapIPv6RoutingTables(const std::function<void(IPv6RoutingTable&)>& procedure)
{
    mapIPv6RoutingTables(*cSimulation::getActiveSimulation(), procedure);
}

void mapIPv6RoutingTables(const cSimulation& simulation, const std::function<void(IPv6RoutingTable&)>& procedure)
{
    mapIPv6RoutingTables(*simulation.getSystemModule(), procedure);
}

void mapIPv6RoutingTables(const cModule& module, const std::function<void(IPv6RoutingTable&)>& procedure)
{
    mapSubmodulesRecursively(module, [&] (cModule& module) {
        if (auto ipv6RoutingTable = dynamic_cast<IPv6RoutingTable *>(&module))
            procedure(*ipv6RoutingTable);
    });
}

// network -> interface table
void mapInterfaceTables(const std::function<void(IInterfaceTable&)>& procedure)
{
    mapInterfaceTables(*cSimulation::getActiveSimulation(), procedure);
}

void mapInterfaceTables(const cSimulation& simulation, const std::function<void(IInterfaceTable&)>& procedure)
{
    mapInterfaceTables(*simulation.getSystemModule(), procedure);
}

void mapInterfaceTables(const cModule& module, const std::function<void(IInterfaceTable&)>& procedure)
{
    mapSubmodulesRecursively(module, [&] (cModule& module) {
        if (auto interfaceTable = dynamic_cast<IInterfaceTable *>(&module))
            procedure(*interfaceTable);
    });
}

// network -> IPv4 address
void mapIPv4Addresses(const std::function<void(IPv4Address&)>& procedure)
{
    mapIPv4Addresses(*cSimulation::getActiveSimulation(), procedure);
}

void mapIPv4Addresses(const cSimulation& simulation, const std::function<void(IPv4Address&)>& procedure)
{
    mapIPv4Addresses(*simulation.getSystemModule(), procedure);
}

void mapIPv4Addresses(const cModule& module, const std::function<void(IPv4Address&)>& procedure)
{
    mapNetworkInterfaces(module, [&] (InterfaceEntry& networkInterface) {
        auto address = networkInterface.getIPv4Address();
        if (!address.isUnspecified())
            procedure(address);
    });
}

// network -> IPv6 address
void mapIPv6Addresses(const std::function<void(IPv6Address&)>& procedure)
{
    mapIPv6Addresses(*cSimulation::getActiveSimulation(), procedure);
}

void mapIPv6Addresses(const cSimulation& simulation, const std::function<void(IPv6Address&)>& procedure)
{
    mapIPv6Addresses(*simulation.getSystemModule(), procedure);
}

void mapIPv6Addresses(const cModule& module, const std::function<void(IPv6Address&)>& procedure)
{
    mapNetworkInterfaces(module, [&] (InterfaceEntry& networkInterface) {
        auto ipv6Data = networkInterface.ipv6Data();
        for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
            auto address = ipv6Data->getAddress(0);
            if (!address.isUnspecified())
                procedure(address);
        }
    });
}

// network -> MAC address
void mapMACAddresses(const std::function<void(MACAddress&)>& procedure)
{
    mapMACAddresses(*cSimulation::getActiveSimulation(), procedure);
}

void mapMACAddresses(const cSimulation& simulation, const std::function<void(MACAddress&)>& procedure)
{
    mapMACAddresses(*simulation.getSystemModule(), procedure);
}

void mapMACAddresses(const cModule& module, const std::function<void(MACAddress&)>& procedure)
{
    mapNetworkInterfaces(module, [&] (InterfaceEntry& networkInterface) {
        auto address = networkInterface.getMacAddress();
        if (!address.isUnspecified())
            procedure(address);
    });
}


// network node -> network interface
InterfaceEntry *findDefaultNetworkInterface(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto interfaceTable = findDefaultInterfaceTable(networkNode);
    if (interfaceTable == nullptr)
        return nullptr;
    else
        return findDefaultNetworkInterface(*interfaceTable);
}

InterfaceEntry& getDefaultNetworkInterface(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto networkInterface = findDefaultNetworkInterface(networkNode);
    if (networkInterface == nullptr)
        throw cRuntimeError("Default network interface not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return *networkInterface;
}

// network node -> IPv4 routing table
IIPv4RoutingTable *findDefaultIPv4RoutingTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
#ifdef WITH_IPv4
    return dynamic_cast<IIPv4RoutingTable *>(networkNode.getModuleByPath(".ipv4.routingTable"));
#else // ifdef WITH_IPv4
    return nullptr;
#endif // ifdef WITH_IPv4
}

IIPv4RoutingTable& getDefaultIPv4RoutingTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto routingTable = findDefaultIPv4RoutingTable(networkNode);
    if (routingTable == nullptr)
        throw cRuntimeError("Default IPv4 routing table not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return *routingTable;
}

// network node -> IPv6 routing table
IPv6RoutingTable *findDefaultIPv6RoutingTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
#ifdef WITH_IPv6
    return dynamic_cast<IPv6RoutingTable *>(networkNode.getModuleByPath(".ipv6.routingTable"));
#else // ifdef WITH_IPv6
    return nullptr;
#endif // ifdef WITH_IPv6
}

IPv6RoutingTable& getDefaultIPv6RoutingTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto routingTable = findDefaultIPv6RoutingTable(networkNode);
    if (routingTable == nullptr)
        throw cRuntimeError("Default IPv6 routing table not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return *routingTable;
}

// network node -> interface table
IInterfaceTable *findDefaultInterfaceTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    return dynamic_cast<IInterfaceTable *>(networkNode.getSubmodule("interfaceTable"));
}

IInterfaceTable *getDefaultInterfaceTable(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto interfaceTable = findDefaultInterfaceTable(networkNode);
    if (interfaceTable == nullptr)
        throw cRuntimeError("Default interface table not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return interfaceTable;
}

// network node -> IPv4 address
IPv4Address findDefaultIPv4Address(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto networkInterface = findDefaultNetworkInterface(networkNode);
    if (networkInterface == nullptr)
        return IPv4Address();
    else
        return networkInterface->getIPv4Address();
}

IPv4Address getDefaultIPv4Address(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto address = findDefaultIPv4Address(networkNode);
    if (address.isUnspecified())
        throw cRuntimeError("Default IPv4 address not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return address;
}

// network node -> IPv6 address
IPv6Address findDefaultIPv6Address(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto networkInterface = findDefaultNetworkInterface(networkNode);
    if (networkInterface == nullptr)
        return IPv6Address();
    else
        return networkInterface->ipv6Data()->getAddress(0);
}

IPv6Address getDefaultIPv6Address(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto address = findDefaultIPv6Address(networkNode);
    if (address.isUnspecified())
        throw cRuntimeError("Default IPv6 address not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return address;
}

// network node -> MAC address
MACAddress findDefaultMACAddress(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto networkInterface = findDefaultNetworkInterface(networkNode);
    if (networkInterface == nullptr)
        return MACAddress();
    else
        return networkInterface->getMacAddress();
}

MACAddress getDefaultMACAddress(const cModule& networkNode)
{
    assert(isNetworkNode(&networkNode));
    auto address = findDefaultMACAddress(networkNode);
    if (address.isUnspecified())
        throw cRuntimeError("Default MAC address not found: network node = %s", networkNode.getFullPath().c_str());
    else
        return address;
}


// network interface -> network node
cModule& getNetworkNode(const InterfaceEntry& networkInterface)
{
    return *getContainingNode(check_and_cast<const cModule *>(&networkInterface));
}
// network interface -> IPv6 address
void mapIPv6Addresses(const InterfaceEntry& networkInterface, const std::function<void(IPv6Address&)>& procedure)
{
    auto ipv6Data = networkInterface.ipv6Data();
    for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
        auto address = ipv6Data->getAddress(i);
        if (!address.isUnspecified())
            procedure(address);
    }
}


// IPv4 routing table -> network node
cModule& getNetworkNode(const IIPv4RoutingTable& routingTable)
{
    return *getContainingNode(check_and_cast<const cModule *>(&routingTable));
}


// IPv6 routing table -> network node
cModule& getNetworkNode(const IPv6RoutingTable& routingTable)
{
    return *getContainingNode(check_and_cast<const cModule *>(&routingTable));
}


// interface table -> network node
cModule& getNetworkNode(const IInterfaceTable& interfaceTable)
{
    return *getContainingNode(check_and_cast<const cModule *>(&interfaceTable));
}

// interface table -> network interface
InterfaceEntry *findDefaultNetworkInterface(const IInterfaceTable& interfaceTable)
{
    InterfaceEntry *defaultNetworkInterface = nullptr;
    mapNetworkInterfaces(interfaceTable, [&] (InterfaceEntry& networkInterface) {
        if (!networkInterface.isLoopback()) {
            if (defaultNetworkInterface != nullptr)
                throw cRuntimeError("More than one default (non-loopback) interface found: interface table = %s", interfaceTable.getFullPath().c_str());
            else
                defaultNetworkInterface = &networkInterface;
        }
    });
    return defaultNetworkInterface;
}

InterfaceEntry& getDefaultNetworkInterface(const IInterfaceTable& interfaceTable)
{
    auto networkInterface = findDefaultNetworkInterface(interfaceTable);
    if (networkInterface == nullptr)
        throw cRuntimeError("Default network interface not found: interface table = %s", interfaceTable.getFullPath().c_str());
    else
        return *networkInterface;
}

void mapNetworkInterfaces(const IInterfaceTable& interfaceTable, const std::function<void(InterfaceEntry&)>& procedure)
{
    for (int i = 0; i < interfaceTable.getNumInterfaces(); i++) {
        auto interfaceEntry = interfaceTable.getInterface(i);
        if (interfaceEntry != nullptr)
            procedure(*interfaceEntry);
    }
}

// interface table -> IPv4 address
IPv4Address findDefaultIPv4Address(const IInterfaceTable& interfaceTable)
{
    auto networkInterface = findDefaultNetworkInterface(interfaceTable);
    if (networkInterface == nullptr)
        return IPv4Address();
    else
        return networkInterface->getIPv4Address();
}

IPv4Address getDefaultIPv4Address(const IInterfaceTable& interfaceTable)
{
    auto address = findDefaultIPv4Address(interfaceTable);
    if (address.isUnspecified())
        throw cRuntimeError("Default IPv4 address not found: interface table = %s", interfaceTable.getFullPath().c_str());
    else
        return address;
}

void mapIPv4Addresses(const IInterfaceTable& interfaceTable, const std::function<void(IPv4Address&)>& procedure)
{
    mapNetworkInterfaces(interfaceTable, [&] (InterfaceEntry& networkInterface) {
        auto address = networkInterface.getIPv4Address();
        if (!address.isUnspecified())
            procedure(address);
    });
}

// interface table -> IPv6 address
IPv6Address findDefaultIPv6Address(const IInterfaceTable& interfaceTable)
{
    auto networkInterface = findDefaultNetworkInterface(interfaceTable);
    if (networkInterface == nullptr)
        return IPv6Address();
    else
        return networkInterface->ipv6Data()->getAddress(0);
}

IPv6Address getDefaultIPv6Address(const IInterfaceTable& interfaceTable)
{
    auto address = findDefaultIPv6Address(interfaceTable);
    if (address.isUnspecified())
        throw cRuntimeError("Default IPv6 address not found: interface table = %s", interfaceTable.getFullPath().c_str());
    else
        return address;
}

void mapIPv6Addresses(const IInterfaceTable& interfaceTable, const std::function<void(IPv6Address&)>& procedure)
{
    mapNetworkInterfaces(interfaceTable, [&] (InterfaceEntry& networkInterface) {
        mapIPv6Addresses(networkInterface, procedure);
    });
}

// interface table -> MAC address
MACAddress findDefaultMACAddress(const IInterfaceTable& interfaceTable)
{
    auto networkInterface = findDefaultNetworkInterface(interfaceTable);
    if (networkInterface == nullptr)
        return MACAddress();
    else
        return networkInterface->getMacAddress();
}

MACAddress getDefaultMACAddress(const IInterfaceTable& interfaceTable)
{
    auto address = findDefaultMACAddress(interfaceTable);
    if (address.isUnspecified())
        throw cRuntimeError("Default MAC address not found: interface table = %s", interfaceTable.getFullPath().c_str());
    else
        return address;
}

void mapMACAddresses(const IInterfaceTable& interfaceTable, const std::function<void(MACAddress&)>& procedure)
{
    mapNetworkInterfaces(interfaceTable, [&] (InterfaceEntry& networkInterface) {
        auto address = networkInterface.getMacAddress();
        if (!address.isUnspecified())
            procedure(address);
    });
}


// IPv4 address -> network node
cModule *findSingleNetworkNode(const IPv4Address& address)
{
    cModule *foundNetworkNode = nullptr;
    mapNetworkNodes(address, [&] (cModule& networkNode) {
        if (foundNetworkNode != nullptr)
            throw cRuntimeError("More than one network node found: IPv4 address = %s", address.str().c_str());
        else
            foundNetworkNode = &networkNode;
    });
    return foundNetworkNode;
}

cModule& getSingleNetworkNode(const IPv4Address& address)
{
    auto networkNode = findSingleNetworkNode(address);
    if (networkNode == nullptr)
        throw cRuntimeError("Single network node not found: IPv4 address = %s", address.str().c_str());
    else
        return *networkNode;
}

void mapNetworkNodes(const IPv4Address& address, const std::function<void(cModule&)>& procedure)
{
    mapNetworkNodes([&] (cModule& networkNode) {
       mapNetworkInterfaces(networkNode, [&] (InterfaceEntry& networkInterface) {
           if (networkInterface.getIPv4Address() == address)
               procedure(networkNode);
       });
    });
}

// IPv4 address -> network interface
InterfaceEntry *findSingleNetworkInterface(const IPv4Address& address)
{
    InterfaceEntry *foundNetworkInterface = nullptr;
    mapNetworkInterfaces(address, [&] (InterfaceEntry& networkInterface) {
        if (foundNetworkInterface != nullptr)
            throw cRuntimeError("More than one network interface found: MAC address = %s", address.str().c_str());
        else
            foundNetworkInterface = &networkInterface;
    });
    return foundNetworkInterface;
}

InterfaceEntry& getSingleNetworkInterface(const IPv4Address& address)
{
    auto networkInterface = findSingleNetworkInterface(address);
    if (networkInterface == nullptr)
        throw cRuntimeError("Single network interface not found: IPv4 address = %s", address.str().c_str());
    else
        return *networkInterface;
}

void mapNetworkInterfaces(const IPv4Address& address, const std::function<void(InterfaceEntry&)>& procedure)
{
    mapNetworkInterfaces([&] (InterfaceEntry& networkInterface) {
        if (networkInterface.getIPv4Address() == address)
            procedure(networkInterface);
    });
}


// IPv6 address -> network node
cModule *findSingleNetworkNode(const IPv6Address& address)
{
    cModule *foundNetworkNode = nullptr;
    mapNetworkNodes(address, [&] (cModule& networkNode) {
        if (foundNetworkNode != nullptr)
            throw cRuntimeError("More than one network node found: IPv6 address = %s", address.str().c_str());
        else
            foundNetworkNode = &networkNode;
    });
    return foundNetworkNode;
}

cModule& getSingleNetworkNode(const IPv6Address& address)
{
    auto networkNode = findSingleNetworkNode(address);
    if (networkNode == nullptr)
        throw cRuntimeError("Single network node not found: IPv6 address = %s", address.str().c_str());
    else
        return *networkNode;
}

void mapNetworkNodes(const IPv6Address& address, const std::function<void(cModule&)>& procedure)
{
    mapNetworkNodes([&] (cModule& networkNode) {
       mapNetworkInterfaces(networkNode, [&] (InterfaceEntry& networkInterface) {
           auto ipv6Data = networkInterface.ipv6Data();
           for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
               auto address = ipv6Data->getAddress(0);
               if (!address.isUnspecified())
                   procedure(networkNode);
           }
       });
    });
}

// IPv6 address -> network interface
InterfaceEntry *findSingleNetworkInterface(const IPv6Address& address)
{
    InterfaceEntry *foundNetworkInterface = nullptr;
    mapNetworkInterfaces(address, [&] (InterfaceEntry& networkInterface) {
        if (foundNetworkInterface != nullptr)
            throw cRuntimeError("More than one network interface found: MAC address = %s", address.str().c_str());
        else
            foundNetworkInterface = &networkInterface;
    });
    return foundNetworkInterface;
}

InterfaceEntry& getSingleNetworkInterface(const IPv6Address& address)
{
    auto networkInterface = findSingleNetworkInterface(address);
    if (networkInterface == nullptr)
        throw cRuntimeError("Single network interface not found: IPv6 address = %s", address.str().c_str());
    else
        return *networkInterface;
}

void mapNetworkInterfaces(const IPv6Address& address, const std::function<void(InterfaceEntry&)>& procedure)
{
    mapNetworkInterfaces([&] (InterfaceEntry& networkInterface) {
        auto ipv6Data = networkInterface.ipv6Data();
        for (int i = 0; i < ipv6Data->getNumAddresses(); i++) {
            if (ipv6Data->getAddress(0) == address)
                procedure(networkInterface);
        }
    });
}


// MAC address -> network node
cModule *findSingleNetworkNode(const MACAddress& address)
{
    cModule *foundNetworkNode = nullptr;
    mapNetworkNodes(address, [&] (cModule& networkNode) {
        if (foundNetworkNode != nullptr)
            throw cRuntimeError("More than one network node found: MAC address = %s", address.str().c_str());
        else
            foundNetworkNode = &networkNode;
    });
    return foundNetworkNode;
}

cModule& getSingleNetworkNode(const MACAddress& address)
{
    auto networkNode = findSingleNetworkNode(address);
    if (networkNode == nullptr)
        throw cRuntimeError("Single network node not found: MAC address = %s", address.str().c_str());
    else
        return *networkNode;
}

void mapNetworkNodes(const MACAddress& address, const std::function<void(cModule&)>& procedure)
{
    mapNetworkNodes([&] (cModule& networkNode) {
       mapNetworkInterfaces(networkNode, [&] (InterfaceEntry& networkInterface) {
           if (networkInterface.getMacAddress() == address)
               procedure(networkNode);
       });
    });
}

// MAC address -> network interface
InterfaceEntry *findSingleNetworkInterface(const MACAddress& address)
{
    InterfaceEntry *foundNetworkInterface = nullptr;
    mapNetworkInterfaces(address, [&] (InterfaceEntry& networkInterface) {
        if (foundNetworkInterface != nullptr)
            throw cRuntimeError("More than one network interface found: MAC address = %s", address.str().c_str());
        else
            foundNetworkInterface = &networkInterface;
    });
    return foundNetworkInterface;
}

InterfaceEntry& getSingleNetworkInterface(const MACAddress& address)
{
    auto networkInterface = findSingleNetworkInterface(address);
    if (networkInterface == nullptr)
        throw cRuntimeError("Single network interface not found: MAC address = %s", address.str().c_str());
    else
        return *networkInterface;
}

void mapNetworkInterfaces(const MACAddress& address, const std::function<void(InterfaceEntry&)>& procedure)
{
    mapNetworkInterfaces([&] (InterfaceEntry& networkInterface) {
        if (networkInterface.getMacAddress() == address)
            procedure(networkInterface);
    });
}

} // namespace
