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

#include "inet/networklayer/common/IInterfaceTable.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_GENERIC
#include "inet/networklayer/generic/GenericNetworkProtocolInterfaceData.h"
#endif // ifdef WITH_GENERIC

namespace inet {

Register_Abstract_Class(InterfaceEntryChangeDetails);
Register_Abstract_Class(InterfaceEntry);

void InterfaceProtocolData::changed(simsignal_t signalID, int fieldId)
{
    // notify the containing InterfaceEntry that something changed
    if (ownerp)
        ownerp->changed(signalID, fieldId);
}

std::string InterfaceEntryChangeDetails::info() const
{
    return ie->info();
}

std::string InterfaceEntryChangeDetails::detailedInfo() const
{
    std::stringstream out;
    out << ie->detailedInfo() << " changed field: " << field << "\n";
    return out.str();
}

InterfaceEntry::InterfaceEntry(cModule *ifmod)
{
    ownerp = NULL;
    interfaceModule = ifmod;

    nwLayerGateIndex = -1;
    nodeOutputGateId = -1;
    nodeInputGateId = -1;

    mtu = 0;

    state = UP;
    carrier = true;
    broadcast = false;
    multicast = false;
    pointToPoint = false;
    loopback = false;
    datarate = 0;

    ipv4data = NULL;
    ipv6data = NULL;
    genericNetworkProtocolData = NULL;
    isisdata = NULL;
    trilldata = NULL;
    ieee8021ddata = NULL;
    estimateCostProcessArray.clear();
}

InterfaceEntry::~InterfaceEntry()
{
    resetInterface();
}

std::string InterfaceEntry::info() const
{
    std::stringstream out;
    out << (getName()[0] ? getName() : "*");
    if (getNetworkLayerGateIndex() == -1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "  MTU:" << getMTU();
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
        out << " " << ipv4data->info();
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data)
        out << " " << ipv6data->info();
#endif // ifdef WITH_IPv6
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->info(); // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->info(); // Khmm...
    if (ieee8021ddata)
        out << " " << ((InterfaceProtocolData *)ieee8021ddata)->info(); // Khmm...
    return out.str();
}

std::string InterfaceEntry::detailedInfo() const
{
    std::stringstream out;
    out << "name:" << (getName()[0] ? getName() : "*");
    if (getNetworkLayerGateIndex() == -1)
        out << "  on:-";
    else
        out << "  on:nwLayer.ifOut[" << getNetworkLayerGateIndex() << "]";
    out << "MTU: " << getMTU() << " \t";
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
    if (isisdata)
        out << " " << ((InterfaceProtocolData *)isisdata)->detailedInfo() << "\n"; // Khmm...
    if (trilldata)
        out << " " << ((InterfaceProtocolData *)trilldata)->detailedInfo() << "\n"; // Khmm...
    if (ieee8021ddata)
        out << " " << ((InterfaceProtocolData *)ieee8021ddata)->detailedInfo() << "\n"; // Khmm...
    return out.str();
}

std::string InterfaceEntry::getFullPath() const
{
    return ownerp == NULL ? getFullName() : ownerp->getHostModule()->getFullPath() + "." + getFullName();
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
    ipv4data = NULL;
#else // ifdef WITH_IPv4
    if (ipv4data)
        throw cRuntimeError(this, "Model error: ipv4data filled, but INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (ipv6data && ipv6data->ownerp == this)
        delete ipv6data;
    ipv6data = NULL;
#else // ifdef WITH_IPv6
    if (ipv6data)
        throw cRuntimeError(this, "Model error: ipv6data filled, but INET was compiled without IPv6 support");
#endif // ifdef WITH_IPv6
    if (isisdata && ((InterfaceProtocolData *)isisdata)->ownerp == this)
        delete (InterfaceProtocolData *)isisdata;
    isisdata = NULL;
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this)
        delete (InterfaceProtocolData *)trilldata;
    trilldata = NULL;
    if (ieee8021ddata && ((InterfaceProtocolData *)ieee8021ddata)->ownerp == this)
        delete (InterfaceProtocolData *)ieee8021ddata;
    ieee8021ddata = NULL;
}

void InterfaceEntry::setGenericNetworkProtocolData(GenericNetworkProtocolInterfaceData *p)
{
#ifdef WITH_GENERIC
    if (genericNetworkProtocolData && genericNetworkProtocolData->ownerp == this)
        delete ipv4data;
    genericNetworkProtocolData = p;
    p->ownerp = this;
    configChanged(F_GENERIC_DATA);
#else // ifdef WITH_GENERIC
    throw cRuntimeError(this, "setGenericNetworkProtocolData(): INET was compiled without Generic Network Layer support");
#endif // ifdef WITH_GENERIC
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
#ifdef WITH_GENERIC
    if (genericNetworkProtocolData)
        return genericNetworkProtocolData->getAddress();
#endif // ifdef WITH_GENERIC
    return getModulePathAddress();
}

void InterfaceEntry::setIPv4Data(IPv4InterfaceData *p)
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

void InterfaceEntry::setIPv6Data(IPv6InterfaceData *p)
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

void InterfaceEntry::setTRILLInterfaceData(TRILLInterfaceData *p)
{
    if (trilldata && ((InterfaceProtocolData *)trilldata)->ownerp == this) // Khmm...
        delete (InterfaceProtocolData *)trilldata; // Khmm...
    trilldata = p;
    ((InterfaceProtocolData *)p)->ownerp = this;    // Khmm...
    configChanged(F_TRILL_DATA);
}

void InterfaceEntry::setISISInterfaceData(ISISInterfaceData *p)
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
        estimateCostProcessArray.resize(position + 1, NULL);
    }
    if (estimateCostProcessArray[position] != NULL)
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
    return NULL;
}

void InterfaceEntry::joinMulticastGroup(const L3Address& address) const
{
    switch (address.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4:
            ipv4Data()->joinMulticastGroup(address.toIPv4());
            break;

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            // TODO
            // ipv6Data()->joinMulticastGroup(address.toIPv6());
            break;

#endif // ifdef WITH_IPv6
#ifdef WITH_GENERIC
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            getGenericNetworkProtocolData()->joinMulticastGroup(address);
            break;

#endif // ifdef WITH_GENERIC
        default:
            throw cRuntimeError("Unknown address type");
    }
}

static void toIPv4AddressVector(const std::vector<L3Address>& addresses, std::vector<IPv4Address>& result)
{
    result.reserve(addresses.size());
    for (unsigned int i = 0; i < addresses.size(); ++i)
        result.push_back(addresses[i].toIPv4());
}

void InterfaceEntry::changeMulticastGroupMembership(const L3Address& multicastAddress,
        McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
        McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList)
{
    switch (multicastAddress.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4: {
            std::vector<IPv4Address> oldIPv4SourceList, newIPv4SourceList;
            toIPv4AddressVector(oldSourceList, oldIPv4SourceList);
            toIPv4AddressVector(newSourceList, newIPv4SourceList);
            ipv4Data()->changeMulticastGroupMembership(multicastAddress.toIPv4(),
                    oldFilterMode, oldIPv4SourceList, newFilterMode, newIPv4SourceList);
            break;
        }

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            // TODO
            break;

#endif // ifdef WITH_IPv6
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            // TODO
            break;

        default:
            throw cRuntimeError("Unknown address type");
    }
}

} // namespace inet

