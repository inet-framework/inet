//
// Copyright (C) 2005 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/networklayer/common/NetworkInterface.h"

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"
#include "inet/common/SubmoduleLayout.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"

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

Register_Abstract_Class(NetworkInterfaceChangeDetails);
Define_Module(NetworkInterface);

std::ostream& operator <<(std::ostream& o, NetworkInterface::State s)
{
    switch (s) {
        case NetworkInterface::UP: o << "UP"; break;
        case NetworkInterface::DOWN: o << "DOWN"; break;
        case NetworkInterface::GOING_UP: o << "GOING_UP"; break;
        case NetworkInterface::GOING_DOWN: o << "GOING_DOWN"; break;
        default: o << (int)s;
    }
    return o;
}

void InterfaceProtocolData::changed(simsignal_t signalID, int fieldId)
{
    // notify the containing NetworkInterface that something changed
    if (ownerp)
        ownerp->changed(signalID, fieldId);
}

std::string NetworkInterfaceChangeDetails::str() const
{
    std::stringstream out;
    out << ie->str() << " changed field: " << field << "\n";
    return out.str();
}

NetworkInterface::NetworkInterface()
{
}

NetworkInterface::~NetworkInterface()
{
    resetInterface();
}

void NetworkInterface::clearProtocolDataSet()
{
    std::vector<int> ids;
    int n = protocolDataSet.getNumTags();
    ids.reserve(n);
    for (int i=0; i < n; i++)
        ids[i] = static_cast<const InterfaceProtocolData *>(protocolDataSet.getTag(i))->id;
    protocolDataSet.clearTags();
    for (int i=0; i < n; i++)
        changed(interfaceConfigChangedSignal, ids[i]);
}

void NetworkInterface::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        upperLayerOut = gate("upperLayerOut");
        if (hasGate("phys$i")) {
            rxIn = gate("phys$i");
            rxTransmissionChannel = rxIn->findIncomingTransmissionChannel();
            if (rxTransmissionChannel != nullptr)
                rxTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
        if (hasGate("phys$o")) {
            txOut = gate("phys$o");
            txTransmissionChannel = txOut->findTransmissionChannel();
            if (txTransmissionChannel != nullptr)
                txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        }
        consumer = findConnectedModule<IPassivePacketSink>(upperLayerOut);
        interfaceTable.reference(this, "interfaceTableModule", false);
        setInterfaceName(utils::stripnonalnum(getFullName()).c_str());
        setCarrier(computeCarrier());
        setDatarate(computeDatarate());
        WATCH(mtu);
        WATCH(state);
        WATCH(carrier);
        WATCH(broadcast);
        WATCH(multicast);
        WATCH(pointToPoint);
        WATCH(loopback);
        WATCH(datarate);
        WATCH(macAddr);
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP) {
            state = UP;
            carrier = true;
        }
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        if (hasPar("protocol")) {
            const char *protocolName = par("protocol");
            if (*protocolName != '\0')
                protocol = Protocol::getProtocol(protocolName);
        }
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
        if (hasPar("mtu"))
            setMtu(par("mtu"));
        if (hasPar("pointToPoint"))
            setPointToPoint(par("pointToPoint"));
        if (interfaceTable)
            interfaceTable->addInterface(this);
        inet::registerInterface(*this, gate("upperLayerIn"), gate("upperLayerOut"));
    }
    else if (stage == INITSTAGE_LAST)
        layoutSubmodulesWithoutGates(this);
}

void NetworkInterface::arrived(cMessage *message, cGate *gate, const SendOptions& options, simtime_t time)
{
    if (gate == upperLayerOut && message->isPacket()) {
        auto packet = check_and_cast<Packet *>(message);
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
    }
    cModule::arrived(message, gate, options, time);
}

void NetworkInterface::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
    consumer->pushPacket(packet, upperLayerOut->getPathEndGate());
}

void NetworkInterface::refreshDisplay() const
{
    updateDisplayString();
}

void NetworkInterface::updateDisplayString() const
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

void NetworkInterface::handleParameterChange(const char *name)
{
    if (name != nullptr && !strcmp(name, "bitrate"))
        setDatarate(computeDatarate());
}

void NetworkInterface::receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details)
{
    Enter_Method("receiveSignal");
    if (signal == POST_MODEL_CHANGE) {
        if (auto notification = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if (rxIn == notification->pathEndGate || txOut == notification->pathStartGate) {
                rxTransmissionChannel = rxIn->findIncomingTransmissionChannel();
                txTransmissionChannel = txOut->findTransmissionChannel();
                if (rxTransmissionChannel != nullptr && !rxTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
                    rxTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
                if (txTransmissionChannel != nullptr && !txTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
                    txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
                setCarrier(computeCarrier());
            }
        }
        else if (auto notification = dynamic_cast<cPostPathCutNotification *>(obj)) {
            if (rxIn == notification->pathEndGate || txOut == notification->pathStartGate) {
                rxTransmissionChannel = nullptr;
                txTransmissionChannel = nullptr;
                setCarrier(computeCarrier());
            }
        }
        else if (auto notification = dynamic_cast<cPostParameterChangeNotification *>(obj)) {
            auto owner = notification->par->getOwner();
            if (owner == rxTransmissionChannel || owner == txTransmissionChannel) {
                setCarrier(computeCarrier());
                setDatarate(computeDatarate());
            }
        }
    }
}

double NetworkInterface::computeDatarate() const
{
    double moduleDatarate = 0;
    if (hasPar("bitrate"))
        moduleDatarate = par("bitrate");
    double channelDatarate = 0;
    if (txTransmissionChannel != nullptr && txTransmissionChannel->hasPar("datarate"))
        channelDatarate = txTransmissionChannel->par("datarate");
    if (moduleDatarate != 0 && channelDatarate != 0 && moduleDatarate != channelDatarate)
        throw cRuntimeError("Wired network interface datarate is set on both the network interface module and on the corresponding transmission channel and the two values are different");
    return moduleDatarate != 0 ? moduleDatarate : channelDatarate;
}

bool NetworkInterface::computeCarrier() const
{
    if (isWireless())
        return true;
    else {
        bool connected = rxTransmissionChannel != nullptr && txTransmissionChannel != nullptr;
        bool disabled = !connected || rxTransmissionChannel->isDisabled() || txTransmissionChannel->isDisabled();
        return connected && !disabled;
    }
}

std::string NetworkInterface::str() const
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

std::string NetworkInterface::getInterfaceFullPath() const
{
    return !interfaceTable ? getInterfaceName() : interfaceTable->getHostModule()->getFullPath() + "." + getInterfaceName();
}

void NetworkInterface::changed(simsignal_t signalID, int fieldId)
{
    if (interfaceTable) {
        NetworkInterfaceChangeDetails details(this, fieldId);
        interfaceTable->interfaceChanged(signalID, &details);
    }
}

void NetworkInterface::resetInterface()
{
    protocolDataSet.clearTags();
}

bool NetworkInterface::matchesMacAddress(const MacAddress& address) const
{
    // TODO: add real support for multicast MAC addresses
    return address.isBroadcast() || address.isMulticast() || macAddr == address;
}

const L3Address NetworkInterface::getNetworkAddress() const
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

bool NetworkInterface::hasNetworkAddress(const L3Address& address) const
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

bool NetworkInterface::setEstimateCostProcess(int position, MacEstimateCostProcess *p)
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

MacEstimateCostProcess *NetworkInterface::getEstimateCostProcess(int position)
{
    ASSERT(position >= 0);
    if ((size_t)position < estimateCostProcessArray.size()) {
        return estimateCostProcessArray[position];
    }
    return nullptr;
}

void NetworkInterface::joinMulticastGroup(const L3Address& address)
{
    switch (address.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4:
            getProtocolDataForUpdate<Ipv4InterfaceData>()->joinMulticastGroup(address.toIpv4());
            break;

#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        case L3Address::IPv6:
            getProtocolDataForUpdate<Ipv6InterfaceData>()->joinMulticastGroup(address.toIpv6());
            break;

#endif // ifdef WITH_IPv6
#ifdef WITH_NEXTHOP
        case L3Address::MAC:
        case L3Address::MODULEID:
        case L3Address::MODULEPATH:
            getProtocolDataForUpdate<NextHopInterfaceData>()->joinMulticastGroup(address);
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

void NetworkInterface::changeMulticastGroupMembership(const L3Address& multicastAddress,
        McastSourceFilterMode oldFilterMode, const std::vector<L3Address>& oldSourceList,
        McastSourceFilterMode newFilterMode, const std::vector<L3Address>& newSourceList)
{
    switch (multicastAddress.getType()) {
#ifdef WITH_IPv4
        case L3Address::IPv4: {
            std::vector<Ipv4Address> oldIPv4SourceList, newIPv4SourceList;
            toIpv4AddressVector(oldSourceList, oldIPv4SourceList);
            toIpv4AddressVector(newSourceList, newIPv4SourceList);
            getProtocolDataForUpdate<Ipv4InterfaceData>()->changeMulticastGroupMembership(multicastAddress.toIpv4(),
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

Ipv4Address NetworkInterface::getIpv4Address() const {
#ifdef WITH_IPv4
    auto ipv4data = findProtocolData<Ipv4InterfaceData>();
    return ipv4data == nullptr ? Ipv4Address::UNSPECIFIED_ADDRESS : ipv4data->getIPAddress();
#else
    return Ipv4Address::UNSPECIFIED_ADDRESS;
#endif // ifdef WITH_IPv4
}

Ipv4Address NetworkInterface::getIpv4Netmask() const {
#ifdef WITH_IPv4
    auto ipv4data = findProtocolData<Ipv4InterfaceData>();
    return ipv4data == nullptr ? Ipv4Address::UNSPECIFIED_ADDRESS : ipv4data->getNetmask();
#else
    return Ipv4Address::UNSPECIFIED_ADDRESS;
#endif // ifdef WITH_IPv4
}

void NetworkInterface::setState(State s)
{
    if (state != s)
    {
        state = s;
        // TODO: carrier and UP/DOWN state is independent
        if (state == DOWN)
            setCarrier(false);
        stateChanged(F_STATE);
    }
}

void NetworkInterface::setCarrier(bool b)
{
    if (carrier != b)
    {
        carrier = b;
        stateChanged(F_CARRIER);
    }
}

bool NetworkInterface::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method("handleOperationStage");

    int stage = operation->getCurrentStage();
    if (dynamic_cast<ModuleStartOperation *>(operation)) {
        if (stage == ModuleStartOperation::STAGE_LINK_LAYER) {
            handleStartOperation(operation);
            return true;
        }
    }
    else if (dynamic_cast<ModuleStopOperation *>(operation)) {
        if (stage == ModuleStopOperation::STAGE_LINK_LAYER) {
            handleStopOperation(operation);
            return true;
        }
    }
    else if (dynamic_cast<ModuleCrashOperation *>(operation)) {
        if (stage == ModuleCrashOperation::STAGE_CRASH) {
            handleCrashOperation(operation);
            return true;
        }
    }
    else
        throw cRuntimeError("unaccepted Lifecycle operation: (%s)%s", operation->getClassName(), operation->getName());
    return true;
}

void NetworkInterface::handleStartOperation(LifecycleOperation *operation)
{
    setState(State::UP);
    // TODO: carrier and UP/DOWN state is independent
    setCarrier(true);
}

void NetworkInterface::handleStopOperation(LifecycleOperation *operation)
{
    setState(State::DOWN);
    // TODO: carrier and UP/DOWN state is independent
    setCarrier(false);
}

void NetworkInterface::handleCrashOperation(LifecycleOperation *operation)
{
    setState(State::DOWN);
    // TODO: carrier and UP/DOWN state is independent
    setCarrier(false);
}

NetworkInterface *findContainingNicModule(const cModule *from)
{
    for (cModule *curmod = const_cast<cModule *>(from); curmod; curmod = curmod->getParentModule()) {
        if (auto networkInterface = dynamic_cast<NetworkInterface *>(curmod))
            return networkInterface;
        cProperties *props = curmod->getProperties();
        if (props && props->getAsBool("networkNode"))
            break;
    }
    return nullptr;
}

NetworkInterface *getContainingNicModule(const cModule *from)
{
    auto networkInterface = findContainingNicModule(from);
    if (!networkInterface)
        throw cRuntimeError("getContainingNicModule(): nic module not found (it should be an NetworkInterface class) for module '%s'", from ? from->getFullPath().c_str() : "<nullptr>");
    return networkInterface;
}

} // namespace inet

