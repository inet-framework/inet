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

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/InterfaceEntry.h"


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

std::ostream& operator <<(std::ostream& o, InterfaceEntry::State s)
{
    switch (s) {
        case InterfaceEntry::UP: o << "UP"; break;
        case InterfaceEntry::DOWN: o << "DOWN"; break;
        case InterfaceEntry::GOING_UP: o << "GOING_UP"; break;
        case InterfaceEntry::GOING_DOWN: o << "GOING_DOWN"; break;
        default: o << (int)s;
    }
    return o;
}

void InterfaceProtocolData::changed(simsignal_t signalID, int fieldId)
{
    // notify the containing InterfaceEntry that something changed
    if (ownerp)
        ownerp->changed(signalID, fieldId);
}

std::string InterfaceEntryChangeDetails::str() const
{
    std::stringstream out;
    out << ie->str() << " changed field: " << field << "\n";
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

void InterfaceEntry::clearProtocolDataSet()
{
    std::vector<int> ids;
    int n = protocolDataSet.getNumTags();
    ids.reserve(n);
    for (int i=0; i < n; i++)
        ids[i] = static_cast<InterfaceProtocolData *>(protocolDataSet.getTag(i))->id;
    protocolDataSet.clearTags();
    for (int i=0; i < n; i++)
        changed(interfaceConfigChangedSignal, ids[i]);
}

void InterfaceEntry::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        setInterfaceName(utils::stripnonalnum(getFullName()).c_str());
        WATCH(mtu);
        WATCH(state);
        WATCH(carrier);
        WATCH(broadcast);
        WATCH(multicast);
        WATCH(pointToPoint);
        WATCH(loopback);
        WATCH(datarate);
        WATCH(macAddr);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        if (hasPar("address")) {
            const char *address = par("address");
            if (!strcmp(address, "auto")) {
                setMacAddress(MacAddress::generateAutoAddress());
                par("address").setStringValue(getMacAddress().str());
            }
            else
                setMacAddress(MacAddress(address));
            setInterfaceToken(macAddr.formInterfaceIdentifier());
        }
        if (hasPar("broadcast"))
            setBroadcast(par("broadcast"));
        if (hasPar("multicast"))
            setMulticast(par("multicast"));
        if (hasPar("bitrate"))
            setDatarate(par("bitrate"));
        if (hasPar("mtu"))
            setMtu(par("mtu"));
        if (hasPar("pointToPoint"))
            setPointToPoint(par("pointToPoint"));
        if (auto interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this))
            interfaceTable->addInterface(this);
        inet::registerInterface(*this, gate("upperLayerIn"), gate("upperLayerOut"));
    }
}

void InterfaceEntry::refreshDisplay() const
{
    updateDisplayString();
}

void InterfaceEntry::updateDisplayString() const
{
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'i':
                result = std::to_string(interfaceId);
                break;
            case 'm':
                result = macAddr.str();
                break;
            case 'n':
                result = interfaceName;
                break;
            case 'a':
                result = getNetworkAddress().str();
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

std::string InterfaceEntry::str() const
{
    std::stringstream out;
    out << getInterfaceName();
    out << " ID:" << getInterfaceId();
    out << " MTU:" << getMtu();
    out << ((state == DOWN) ? " DOWN" : " UP");
    if (isLoopback())
        out << " LOOPBACK";
    if (isBroadcast())
        out << " BROADCAST";
    out << (hasCarrier() ? " CARRIER" : " NOCARRIER");
    if (isMulticast())
        out << " MULTICAST";
    if (isPointToPoint())
        out << " POINTTOPOINT";
    out << " macAddr:";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();
    for (int i=0; i<protocolDataSet.getNumTags(); i++)
        out << " " << protocolDataSet.getTag(i)->str();

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
    protocolDataSet.clearTags();
}

const L3Address InterfaceEntry::getNetworkAddress() const
{
#ifdef WITH_IPv4
    if (auto ipv4data = findProtocolData<Ipv4InterfaceData>())
        return ipv4data->getIPAddress();
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    if (auto ipv6data = findProtocolData<Ipv6InterfaceData>())
        return ipv6data->getPreferredAddress();
#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
    if (auto nextHopData = findProtocolData<NextHopInterfaceData>())
        return nextHopData->getAddress();
#endif // ifdef WITH_NEXTHOP
    return getModulePathAddress();
}

bool InterfaceEntry::hasNetworkAddress(const L3Address& address) const
{
    switch(address.getType()) {
    case L3Address::NONE:
        return false;

    case L3Address::IPv4: {
#ifdef WITH_IPv4
        auto ipv4data = findProtocolData<Ipv4InterfaceData>();
        return ipv4data != nullptr && ipv4data->getIPAddress() == address.toIpv4();
#else
        return false;
#endif // ifdef WITH_IPv4
    }
    case L3Address::IPv6: {
#ifdef WITH_IPv6
        auto ipv6data = findProtocolData<Ipv6InterfaceData>();
        return ipv6data != nullptr && ipv6data->hasAddress(address.toIpv6());
#else
        return false;
#endif // ifdef WITH_IPv6
    }
    case L3Address::MAC: {
        return getMacAddress() == address.toMac();
    }
    case L3Address::MODULEID:
    case L3Address::MODULEPATH: {
#ifdef WITH_NEXTHOP
        auto nextHopData = findProtocolData<NextHopInterfaceData>();
        return nextHopData != nullptr && nextHopData->getAddress() == address;
#else
        return false;
#endif // ifdef WITH_NEXTHOP
    }
    default:
        break;
    }
    return false;
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
            getProtocolData<Ipv4InterfaceData>()->joinMulticastGroup(address.toIpv4());
            break;

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            getProtocolData<Ipv6InterfaceData>()->joinMulticastGroup(address.toIpv6());
            break;

#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            getProtocolData<NextHopInterfaceData>()->joinMulticastGroup(address);
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
            getProtocolData<Ipv4InterfaceData>()->changeMulticastGroupMembership(multicastAddress.toIpv4(),
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
    auto ipv4data = findProtocolData<Ipv4InterfaceData>();
    return ipv4data == nullptr ? Ipv4Address::UNSPECIFIED_ADDRESS : ipv4data->getIPAddress();
#else
    return Ipv4Address::UNSPECIFIED_ADDRESS;
#endif // ifdef WITH_IPv4
}

Ipv4Address InterfaceEntry::getIpv4Netmask() const {
#ifdef WITH_IPv4
    auto ipv4data = findProtocolData<Ipv4InterfaceData>();
    return ipv4data == nullptr ? Ipv4Address::UNSPECIFIED_ADDRESS : ipv4data->getNetmask();
#else
    return Ipv4Address::UNSPECIFIED_ADDRESS;
#endif // ifdef WITH_IPv4
}

void InterfaceEntry::setState(State s)
{
    if (state != s)
    {
        state = s;
        if (state == DOWN)
            setCarrier(false);
        stateChanged(F_STATE);
    }
}
void InterfaceEntry::setCarrier(bool b)
{
    ASSERT(!(b && (state == DOWN)));
    if (carrier != b)
    {
        carrier = b;
        stateChanged(F_CARRIER);
    }
}

} // namespace inet

