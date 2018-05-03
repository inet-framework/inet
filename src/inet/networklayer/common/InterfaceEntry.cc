//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <algorithm>
//#include <sstream>



#include "inet/networklayer/common/InterfaceEntry.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

#include "inet/networklayer/contract/IInterfaceTable.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_NEXTHOP
#include "inet/networklayer/nexthop/NextHopInterfaceData.h"
#endif // ifdef WITH_NEXTHOP

namespace inet {

Register_Abstract_Class(InterfaceEntryChangeDetails);
Define_Module(InterfaceEntry);

void InterfaceProtocolData::changed(simsignal_t signalID, int fieldId)
{
    // notify the containing InterfaceEntry that something changed
    if (ownerp)
        ownerp->changed(signalID, fieldId);
}

std::string InterfaceEntryChangeDetails::str() const
{
    return ie->str();
}

std::string InterfaceEntryChangeDetails::detailedInfo() const
{
    std::stringstream out;
    out << ie->detailedInfo() << " changed field: " << field << "\n";
    return out.str();
}

InterfaceEntry::InterfaceEntry()
{
    state = UP;
    carrier = true;
    datarate = 0;
}

InterfaceEntry::~InterfaceEntry()
{
    resetInterface();
}

void InterfaceEntry::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        setInterfaceName(utils::stripnonalnum(getFullName()).c_str());
}

std::string InterfaceEntry::str() const
{
    std::stringstream out;
    out << getInterfaceName();
    out << "  ID:" << getInterfaceId();
    out << "  MTU:" << getMtu();
    if (!isUp())
        out << " DOWN";
    if (isBroadcast())
        out << " BROADCAST";
    if (isMulticast())
        out << " MULTICAST";
    if (isPointToPoint())
        out << " POINTTOPOINT";
    if (isLoopback())
        out << " LOOPBACK";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();

#ifdef WITH_IPv4
    if (ipv4data)
        out << " " << ipv4data->str();
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data)
        out << " " << ipv6data->str();
#endif // ifdef WITH_IPv6
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->str(); // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->str(); // Khmm...
    if (ieee8021ddata)
        out << " " << ((InterfaceProtocolData *)ieee8021ddata)->str(); // Khmm...
    return out.str();
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << getInterfaceName();
    out << "  ID:" << getInterfaceId();
    out << "  MTU: " << getMtu() << " \t";
    if (!isUp())
        out << "DOWN ";
    if (isBroadcast())
        out << "BROADCAST ";
    if (isMulticast())
        out << "MULTICAST ";
    if (isPointToPoint())
        out << "POINTTOPOINT ";
    if (isLoopback())
        out << "LOOPBACK ";
    out << "\n";
    out << "  macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();
    out << "\n";
#ifdef WITH_IPv4
    if (ipv4data)
        out << " " << ipv4data->detailedInfo() << "\n";
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data)
        out << " " << ipv6data->detailedInfo() << "\n";
#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
    if (nextHopData)
        out << " " << nextHopData->detailedInfo() << "\n";
#endif // ifdef WITH_NEXTHOP
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->str() << "\n"; // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->str() << "\n"; // Khmm...
    if (ieee8021ddata)
        out << " " << ((InterfaceProtocolData *)ieee8021ddata)->str() << "\n"; // Khmm...
    return out.str();
}

std::string InterfaceEntry::getInterfaceFullPath() const
{
    return ownerp == nullptr ? getInterfaceName() : ownerp->getHostModule()->getFullPath() + "." + getInterfaceName();
}

void InterfaceEntry::changed(simsignal_t signalID, int fieldId)
{
    if (ownerp) {
        InterfaceEntryChangeDetails details(this, fieldId);
        ownerp->interfaceChanged(signalID, &details);
    }
}

void InterfaceEntry::resetInterface()
{
#ifdef WITH_IPv4
    if (ipv4data && ipv4data->ownerp == this)
        delete ipv4data;
    ipv4data = nullptr;
#else // ifdef WITH_IPv4
    if (ipv4data)
        throw cRuntimeError(this, "Model error: ipv4data filled, but INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data && ipv6data->ownerp == this)
        delete ipv6data;
    ipv6data = nullptr;
#else // ifdef WITH_IPv6
    if (ipv6data)
        throw cRuntimeError(this, "Model error: ipv6data filled, but INET was compiled without IPv6 support");
#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
    if (nextHopData && nextHopData->ownerp == this)
        delete nextHopData;
    nextHopData = nullptr;
#else // ifdef WITH_NEXTHOP
    if (nextHopData)
        throw cRuntimeError(this, "Model error: nextHopProtocolData filled, but INET was compiled without Next Hop Forwarding support");
#endif // ifdef WITH_NEXTHOP
    if (isisdata && ((InterfaceProtocolData *)isisdata)->ownerp == this)
        delete (InterfaceProtocolData *)isisdata;
    isisdata = nullptr;
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this)
        delete (InterfaceProtocolData *)trilldata;
    trilldata = nullptr;
    if (ieee8021ddata && ((InterfaceProtocolData *)ieee8021ddata)->ownerp == this)
        delete (InterfaceProtocolData *)ieee8021ddata;
    ieee8021ddata = nullptr;
}

void InterfaceEntry::setNextHopData(NextHopInterfaceData *p)
{
#ifdef WITH_NEXTHOP
    if (nextHopData && nextHopData->ownerp == this)
        delete nextHopData;
    nextHopData = p;
    p->ownerp = this;
    configChanged(F_NEXTHOP_DATA);
#else // ifdef WITH_NEXTHOP
    throw cRuntimeError(this, "setNextHopProtocolData(): INET was compiled without Next Hop Forwarding support");
#endif // ifdef WITH_NEXTHOP
}

const L3Address InterfaceEntry::getNetworkAddress() const
{
#ifdef WITH_IPv4
    if (ipv4data)
        return ipv4data->getIPAddress();
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data)
        return ipv6data->getPreferredAddress();
#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
    if (nextHopData)
        return nextHopData->getAddress();
#endif // ifdef WITH_NEXTHOP
    return getModulePathAddress();
}

void InterfaceEntry::setIpv4Data(Ipv4InterfaceData *p)
{
#ifdef WITH_IPv4
    if (ipv4data && ipv4data->ownerp == this)
        delete ipv4data;
    ipv4data = p;
    p->ownerp = this;
    configChanged(F_IPV4_DATA);
#else // ifdef WITH_IPv4
    throw cRuntimeError(this, "setIPv4Data(): INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
}

void InterfaceEntry::setIpv6Data(Ipv6InterfaceData *p)
{
#ifdef WITH_IPv6
    if (ipv6data && ipv6data->ownerp == this)
        delete ipv6data;
    ipv6data = p;
    p->ownerp = this;
    configChanged(F_IPV6_DATA);
#else // ifdef WITH_IPv6
    throw cRuntimeError(this, "setIPv6Data(): INET was compiled without IPv6 support");
#endif // ifdef WITH_IPv6
}

void InterfaceEntry::setTrillInterfaceData(TrillInterfaceData *p)
{
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)trilldata; // Khmm...
    trilldata = p;
    ((InterfaceProtocolData *)p)->ownerp = this;    // Khmm...
    configChanged(F_TRILL_DATA);
}

void InterfaceEntry::setIsisInterfaceData(IsisInterfaceData *p)
{
    if (isisdata && ((InterfaceProtocolData *)isisdata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)isisdata; // Khmm...
    isisdata = p;
    ((InterfaceProtocolData *)p)->ownerp = this;    // Khmm...
    configChanged(F_ISIS_DATA);
}

void InterfaceEntry::setIeee8021dInterfaceData(Ieee8021dInterfaceData *p)
{
    if (ieee8021ddata && ((InterfaceProtocolData *)ieee8021ddata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)ieee8021ddata; // Khmm...
    ieee8021ddata = p;
    ((InterfaceProtocolData *)p)->ownerp = this;    // Khmm...
    configChanged(F_IEEE8021D_DATA);
}

bool InterfaceEntry::setEstimateCostProcess(int position, MacEstimateCostProcess *p)
{
    ASSERT(position >= 0);
    if (estimateCostProcessArray.size() <= (size_t)position) {
        estimateCostProcessArray.resize(position + 1, nullptr);
    }
    if (estimateCostProcessArray[position] != nullptr)
        return false;
    estimateCostProcessArray[position] = p;
    return true;
}

MacEstimateCostProcess *InterfaceEntry::getEstimateCostProcess(int position)
{
    ASSERT(position >= 0);
    if ((size_t)position < estimateCostProcessArray.size()) {
        return estimateCostProcessArray[position];
    }
    return nullptr;
}

void InterfaceEntry::joinMulticastGroup(const L3Address& address) const
{
    switch (address.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4:
            ipv4Data()->joinMulticastGroup(address.toIpv4());
            break;

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            ipv6Data()->joinMulticastGroup(address.toIpv6());
            break;

#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            getNextHopData()->joinMulticastGroup(address);
            break;

#endif // ifdef WITH_NEXTHOP
        default:
            throw cRuntimeError("Unknown address type");
    }
}

static void toIpv4AddressVector(const std::vector<L3Address>& addresses, std::vector<Ipv4Address>& result)
{
    result.reserve(addresses.size());
    for (auto & addresse : addresses)
        result.push_back(addresse.toIpv4());
}

void InterfaceEntry::changeMulticastGroupMembership(const L3Address& multicastAddress,
        McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
        McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList)
{
    switch (multicastAddress.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4: {
            std::vector<Ipv4Address> oldIPv4SourceList, newIPv4SourceList;
            toIpv4AddressVector(oldSourceList, oldIPv4SourceList);
            toIpv4AddressVector(newSourceList, newIPv4SourceList);
            ipv4Data()->changeMulticastGroupMembership(multicastAddress.toIpv4(),
                    oldFilterMode, oldIPv4SourceList, newFilterMode, newIPv4SourceList);
            break;
        }

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            // TODO
            throw cRuntimeError("changeMulticastGroupMembership() not implemented for type %s", L3Address::getTypeName(multicastAddress.getType()));
            break;

#endif // ifdef WITH_IPv6
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            // TODO
            throw cRuntimeError("changeMulticastGroupMembership() not implemented for type %s", L3Address::getTypeName(multicastAddress.getType()));
            break;

        default:
            throw cRuntimeError("Unknown address type");
    }
}

Ipv4Address InterfaceEntry::getIpv4Address() const {
#ifdef WITH_IPv4
    return ipv4data == nullptr ? Ipv4Address::UNSPECIFIED_ADDRESS : ipv4data->getIPAddress();
#else
    return Ipv4Address::UNSPECIFIED_ADDRESS;
#endif // ifdef WITH_IPv4
}

} // namespace inet

