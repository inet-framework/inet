//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2008 Alfonso Ariza Quintana (global arp)
// Copyright (C) 2014 OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/networklayer/arp/ipv4/GlobalArp.h"

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif
#ifdef WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#endif

namespace inet {

Define_Module(GlobalArp);

GlobalArp::ArpCache GlobalArp::globalArpCache;
int GlobalArp::globalArpCacheRefCnt = 0;

static std::ostream& operator<<(std::ostream& out, const GlobalArp::ArpCacheEntry& entry)
{
    return out << "MAC:" << entry.networkInterface->getMacAddress();
}

GlobalArp::GlobalArp()
{
    if (++globalArpCacheRefCnt == 1) {
        if (!globalArpCache.empty())
            throw cRuntimeError("Global ARP cache not empty, model error in previous run?");
    }

    interfaceTable = nullptr;
}

GlobalArp::~GlobalArp()
{
    --globalArpCacheRefCnt;
    // delete my entries from the globalArpCache
    for (auto it = globalArpCache.begin(); it != globalArpCache.end(); ) {
        if (it->second->owner == this) {
            auto cur = it++;
            delete cur->second;
            globalArpCache.erase(cur);
        }
        else
            ++it;
    }
}

void GlobalArp::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        const char *addressTypeString = par("addressType");
        if (!strcmp(addressTypeString, "ipv4"))
            addressType = L3Address::IPv4;
        else if (!strcmp(addressTypeString, "ipv6"))
            addressType = L3Address::IPv6;
        else if (!strcmp(addressTypeString, "mac"))
            addressType = L3Address::MAC;
        else if (!strcmp(addressTypeString, "modulepath"))
            addressType = L3Address::MODULEPATH;
        else if (!strcmp(addressTypeString, "moduleid"))
            addressType = L3Address::MODULEID;
        else
            throw cRuntimeError("Unknown address type");
        WATCH_PTRMAP(globalArpCache);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        // register our addresses in the global cache
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            NetworkInterface *networkInterface = interfaceTable->getInterface(i);
            if (!networkInterface->isLoopback()) {
#ifdef WITH_IPv4
                if (auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>()) {
                    Ipv4Address ipv4Address = ipv4Data->getIPAddress();
                    if (!ipv4Address.isUnspecified())
                        ensureCacheEntry(ipv4Address, networkInterface);
                }
#endif
#ifdef WITH_IPv6
                if (auto ipv6Data = networkInterface->findProtocolData<Ipv6InterfaceData>()) {
                    Ipv6Address ipv6Address = ipv6Data->getLinkLocalAddress();
                    if (!ipv6Address.isUnspecified())
                        ensureCacheEntry(ipv6Address, networkInterface);
                }
#endif
#ifdef WITH_NEXTHOP
                if (auto genericData = networkInterface->findProtocolData<NextHopInterfaceData>()) {
                    L3Address address = genericData->getAddress();
                    if (!address.isUnspecified())
                        ensureCacheEntry(address, networkInterface);
                }
#endif
            }
        }
        cModule *node = findContainingNode(this);
        if (node != nullptr) {
            node->subscribe(interfaceIpv4ConfigChangedSignal, this);
            node->subscribe(interfaceIpv6ConfigChangedSignal, this);
        }
    }
}

void GlobalArp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMessage(msg);
    else
        handlePacket(check_and_cast<Packet *>(msg));
}

void GlobalArp::handleSelfMessage(cMessage *msg)
{
    throw cRuntimeError("Model error: unexpected self message");
}

void GlobalArp::handlePacket(Packet *packet)
{
    EV << "Packet " << packet << " arrived, dropping it\n";
    delete packet;
}

void GlobalArp::handleStartOperation(LifecycleOperation *operation)
{
}

void GlobalArp::handleStopOperation(LifecycleOperation *operation)
{
}

void GlobalArp::handleCrashOperation(LifecycleOperation *operation)
{
}

MacAddress GlobalArp::resolveL3Address(const L3Address& address, const NetworkInterface *networkInterface)
{
    Enter_Method("resolveL3Address");
    if (address.isUnicast())
        return mapUnicastAddress(address);
    else if (address.isMulticast())
        return mapMulticastAddress(address);
    else if (address.isBroadcast())
        return MacAddress::BROADCAST_ADDRESS;
    throw cRuntimeError("Address must be one of unicast, multicast, or broadcast");
}

MacAddress GlobalArp::mapUnicastAddress(L3Address address)
{
    switch (address.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4: {
            Ipv4Address ipv4Address = address.toIpv4();
            ArpCache::const_iterator it = globalArpCache.find(ipv4Address);
            if (it != globalArpCache.end())
                return it->second->networkInterface->getMacAddress();
            throw cRuntimeError("GlobalArp does not support dynamic address resolution");
            return MacAddress::UNSPECIFIED_ADDRESS;
        }
#endif
#ifdef WITH_IPv6
        case L3Address::IPv6: {
            Ipv6Address ipv6Address = address.toIpv6();
            ArpCache::const_iterator it = globalArpCache.find(ipv6Address);
            if (it != globalArpCache.end())
                return it->second->networkInterface->getMacAddress();
            throw cRuntimeError("GlobalArp does not support dynamic address resolution");
            return MacAddress::UNSPECIFIED_ADDRESS;
        }
#endif
        case L3Address::MAC:
            return address.toMac();
        case L3Address::MODULEID: {
            auto networkInterface = check_and_cast<NetworkInterface *>(getSimulation()->getModule(address.toModuleId().getId()));
            return networkInterface->getMacAddress();
        }
        case L3Address::MODULEPATH: {
            auto networkInterface = check_and_cast<NetworkInterface *>(getSimulation()->getModule(address.toModulePath().getId()));
            return networkInterface->getMacAddress();
        }
        default:
            throw cRuntimeError("Unknown address type");
    }
}

MacAddress GlobalArp::mapMulticastAddress(L3Address address)
{
    ASSERT(address.isMulticast());

    MacAddress macAddress;
    macAddress.setAddressByte(0, 0x01);
    macAddress.setAddressByte(1, 0x00);
    macAddress.setAddressByte(2, 0x5e);
    // TODO:
    // macAddress.setAddressByte(3, addr.getDByte(1) & 0x7f);
    // macAddress.setAddressByte(4, addr.getDByte(2));
    // macAddress.setAddressByte(5, addr.getDByte(3));
    return macAddress;
}

L3Address GlobalArp::getL3AddressFor(const MacAddress& macAddress) const
{
    Enter_Method("getL3AddressFor");
    switch (addressType) {
#ifdef WITH_IPv4
        case L3Address::IPv4: {
            if (macAddress.isUnspecified())
                return Ipv4Address::UNSPECIFIED_ADDRESS;
            for (ArpCache::const_iterator it = globalArpCache.begin(); it != globalArpCache.end(); it++)
                if (it->second->networkInterface->getMacAddress() == macAddress && it->first.getType() == L3Address::IPv4)
                    return it->first;
            return Ipv4Address::UNSPECIFIED_ADDRESS;
        }
#endif
#ifdef WITH_IPv6
        case L3Address::IPv6: {
            if (macAddress.isUnspecified())
                return Ipv6Address::UNSPECIFIED_ADDRESS;
            for (ArpCache::const_iterator it = globalArpCache.begin(); it != globalArpCache.end(); it++)
                if (it->second->networkInterface->getMacAddress() == macAddress && it->first.getType() == L3Address::IPv6)
                    return it->first;
            return Ipv4Address::UNSPECIFIED_ADDRESS;
        }
#endif
        case L3Address::MAC:
            return L3Address(macAddress);
        case L3Address::MODULEID: {
            if (macAddress.isUnspecified())
                return ModuleIdAddress();
            for (ArpCache::const_iterator it = globalArpCache.begin(); it != globalArpCache.end(); it++)
                if (it->second->networkInterface->getMacAddress() == macAddress && it->first.getType() == L3Address::MODULEID)
                    return it->first;
            return ModuleIdAddress();
        }
        case L3Address::MODULEPATH: {
            if (macAddress.isUnspecified())
                return ModulePathAddress();
            for (ArpCache::const_iterator it = globalArpCache.begin(); it != globalArpCache.end(); it++)
                if (it->second->networkInterface->getMacAddress() == macAddress && it->first.getType() == L3Address::MODULEPATH)
                    return it->first;
            return ModulePathAddress();
        }
        default:
            throw cRuntimeError("Unknown address type");
    }
}

void GlobalArp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("receiveSignal");
    // host associated. Link is up. Change the state to init.
    if (signalID == interfaceIpv4ConfigChangedSignal || signalID == interfaceIpv6ConfigChangedSignal) {
        const NetworkInterfaceChangeDetails *iecd = check_and_cast<const NetworkInterfaceChangeDetails *>(obj);
        NetworkInterface *networkInterface = iecd->getNetworkInterface();
        if (networkInterface->isLoopback())
            return;
        auto it = globalArpCache.begin();
        ArpCacheEntry *entry = nullptr;
#ifdef WITH_IPv4
        if (signalID == interfaceIpv4ConfigChangedSignal) {
            for ( ; it != globalArpCache.end(); ++it) {
                if (it->second->networkInterface == networkInterface && it->first.getType() == L3Address::IPv4)
                    break;
            }
            if (it == globalArpCache.end()) {
                auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>();
                if (!ipv4Data || ipv4Data->getIPAddress().isUnspecified())
                    return; // if the address is not defined it isn't included in the global cache
                entry = new ArpCacheEntry();
                entry->owner = this;
                entry->networkInterface = networkInterface;
            }
            else {
                // actualize
                entry = it->second;
                ASSERT(entry->owner == this);
                globalArpCache.erase(it);
                auto ipv4Data = networkInterface->findProtocolData<Ipv4InterfaceData>();
                if (!ipv4Data || ipv4Data->getIPAddress().isUnspecified()) {
                    delete entry;
                    return;    // if the address is not defined it isn't included in the global cache
                }
            }
            Ipv4Address ipv4Address = networkInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
            auto where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(ipv4Address, entry));
            ASSERT(where->second == entry);
        }
        else
#endif
#ifdef WITH_IPv6
        if (signalID == interfaceIpv6ConfigChangedSignal) {
            for ( ; it != globalArpCache.end(); ++it) {
                if (it->second->networkInterface == networkInterface && it->first.getType() == L3Address::IPv6)
                    break;
            }
            if (it == globalArpCache.end()) {
                auto ipv6Data = networkInterface->findProtocolData<Ipv6InterfaceData>();
                if (ipv6Data == nullptr || ipv6Data->getLinkLocalAddress().isUnspecified())
                    return; // if the address is not defined it isn't included in the global cache
                entry = new ArpCacheEntry();
                entry->owner = this;
                entry->networkInterface = networkInterface;
            }
            else {
                // actualize
                entry = it->second;
                ASSERT(entry->owner == this);
                globalArpCache.erase(it);
                auto ipv6Data = networkInterface->findProtocolData<Ipv6InterfaceData>();
                if (ipv6Data == nullptr || ipv6Data->getLinkLocalAddress().isUnspecified()) {
                    delete entry;
                    return;    // if the address is not defined it isn't included in the global cache
                }
            }
            Ipv6Address ipv6Address = networkInterface->getProtocolData<Ipv6InterfaceData>()->getLinkLocalAddress();
            auto where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(ipv6Address, entry));
            ASSERT(where->second == entry);
        }
        else
#endif
        {}
    }
    else
        throw cRuntimeError("Unknown signal");
}

void GlobalArp::ensureCacheEntry(const L3Address& address, const NetworkInterface *networkInterface)
{
    auto it = globalArpCache.find(address);
    if (it == globalArpCache.end()) {
        ArpCacheEntry *entry = new ArpCacheEntry();
        entry->owner = this;
        entry->networkInterface = networkInterface;
        auto where = globalArpCache.insert(globalArpCache.begin(), std::make_pair(address, entry));
        ASSERT(where->second == entry);
    }
}

} // namespace inet

