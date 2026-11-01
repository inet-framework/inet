//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/vlan/Ieee8021adPolicy.h"

#include "inet/common/ProtocolUtils.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/vlan/VlanIdMaps.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(Ieee8021adPolicy);

static const char *getPortTypeName(Ieee8021adPolicy::PortType type)
{
    switch (type) {
        case Ieee8021adPolicy::PORT_TYPE_ACCESS: return "access";
        case Ieee8021adPolicy::PORT_TYPE_TRUNK: return "trunk";
        case Ieee8021adPolicy::PORT_TYPE_CNP: return "cnp";
        case Ieee8021adPolicy::PORT_TYPE_PNP: return "pnp";
        default: throw cRuntimeError("Unknown port type: %d", type);
    }
}

std::ostream& operator<<(std::ostream& os, const Ieee8021adPolicy::Port& port)
{
    os << getPortTypeName(port.type) << " [";
    bool first = true;
    for (int vlanId : port.vlanIds) {
        os << (first ? "" : ", ") << vlanId;
        first = false;
    }
    return os << "]";
}

void Ieee8021adPolicy::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        vlanTagType = parseVlanTagType(par("vlanTagType"));
        vlanTagProtocol = getVlanTagProtocol(vlanTagType);
        interfaceTable.reference(this, "interfaceTableModule", true);
        upperLayerInGate = gate("upperLayerIn");
        upperLayerOutGate = gate("upperLayerOut");
        lowerLayerInGate = gate("lowerLayerIn");
        lowerLayerOutGate = gate("lowerLayerOut");
        upperLayerSink.reference(upperLayerOutGate, false);
        lowerLayerSink.reference(lowerLayerOutGate, false);
        for (const auto& [interfaceName, value] : check_and_cast<cValueMap *>(par("ports").objectValue())->getFields()) {
            auto port = parsePort(interfaceName.c_str(), value);
            checkPort(interfaceName.c_str(), port);
            ports[interfaceName] = port;
        }
        WATCH_MAP(ports);
        WATCH(numDroppedPackets);
        check_and_cast<cModule *>(interfaceTable.get())->subscribe(interfaceDeletedSignal, this);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPacketOperationSupport(upperLayerInGate);
        checkPacketOperationSupport(upperLayerOutGate);
        checkPacketOperationSupport(lowerLayerInGate);
        checkPacketOperationSupport(lowerLayerOutGate);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        checkInterfaces();
}

void Ieee8021adPolicy::handleMessage(cMessage *message)
{
    if (auto packet = dynamic_cast<Packet *>(message))
        pushPacket(packet, packet->getArrivalGate());
    else if (auto controlMessage = dynamic_cast<Message *>(message)) {
        if (message->getArrivalGate() == upperLayerInGate)
            send(controlMessage, lowerLayerOutGate);
        else if (message->getArrivalGate() == lowerLayerInGate)
            send(controlMessage, upperLayerOutGate);
        else
            throw cRuntimeError("Unknown gate");
    }
    else
        throw cRuntimeError("Unknown message");
}

cGate *Ieee8021adPolicy::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == this->gate("upperLayerIn"))
        return this->gate("lowerLayerOut");
    else if (gate == this->gate("lowerLayerOut"))
        return this->gate("upperLayerIn");
    else if (gate == this->gate("lowerLayerIn"))
        return this->gate("upperLayerOut");
    else if (gate == this->gate("upperLayerOut"))
        return this->gate("lowerLayerIn");
    else
        throw cRuntimeError("Unknown gate");
}

static int parseVlanId(const cComponent *component, const char *interfaceName, const char *fieldName, const cValue& value)
{
    if (value.getType() != cValue::INT)
        throw cRuntimeError(component, "Invalid '%s' in the ports entry for interface '%s': expected an integer VLAN id, got %s", fieldName, interfaceName, value.str().c_str());
    return value.intValue();
}

Ieee8021adPolicy::Port Ieee8021adPolicy::parsePort(const char *interfaceName, const cValue& value) const
{
    auto map = value.getType() == cValue::POINTER ? dynamic_cast<const cValueMap *>(value.objectValue()) : nullptr;
    if (map == nullptr)
        throw cRuntimeError(this, "Invalid ports entry for interface '%s': expected a map with the fields 'type' and 'vid' or 'vids', got %s", interfaceName, value.str().c_str());
    Port port;
    bool hasType = false;
    bool hasVid = false;
    bool hasVids = false;
    for (const auto& [fieldName, fieldValue] : map->getFields()) {
        if (fieldName == "type") {
            std::string type = fieldValue.getType() == cValue::STRING ? fieldValue.stringValue() : "";
            if (type == "access")
                port.type = PORT_TYPE_ACCESS;
            else if (type == "trunk")
                port.type = PORT_TYPE_TRUNK;
            else if (type == "cnp")
                port.type = PORT_TYPE_CNP;
            else if (type == "pnp")
                port.type = PORT_TYPE_PNP;
            else
                throw cRuntimeError(this, "Invalid port type %s in the ports entry for interface '%s': expected \"access\", \"trunk\", \"cnp\" or \"pnp\"", fieldValue.str().c_str(), interfaceName);
            hasType = true;
        }
        else if (fieldName == "vid") {
            port.vlanIds.insert(parseVlanId(this, interfaceName, "vid", fieldValue));
            hasVid = true;
        }
        else if (fieldName == "vids") {
            auto array = fieldValue.getType() == cValue::POINTER ? dynamic_cast<const cValueArray *>(fieldValue.objectValue()) : nullptr;
            if (array == nullptr)
                throw cRuntimeError(this, "Invalid 'vids' in the ports entry for interface '%s': expected a list of VLAN ids, got %s", interfaceName, fieldValue.str().c_str());
            for (int i = 0; i < array->size(); i++)
                port.vlanIds.insert(parseVlanId(this, interfaceName, "vids", array->get(i)));
            hasVids = true;
        }
        else
            throw cRuntimeError(this, "Unknown field '%s' in the ports entry for interface '%s': expected 'type', 'vid' or 'vids'", fieldName.c_str(), interfaceName);
    }
    if (!hasType)
        throw cRuntimeError(this, "The ports entry for interface '%s' has no 'type'", interfaceName);
    if (isUntaggedPort(port.type) && hasVids)
        throw cRuntimeError(this, "The ports entry for interface '%s' has 'vids': a port of type '%s' has one VLAN id, given as 'vid'", interfaceName, getPortTypeName(port.type));
    if (isUntaggedPort(port.type) && !hasVid)
        throw cRuntimeError(this, "The ports entry for interface '%s' has no 'vid': a port of type '%s' needs one", interfaceName, getPortTypeName(port.type));
    if (!isUntaggedPort(port.type) && hasVid)
        throw cRuntimeError(this, "The ports entry for interface '%s' has 'vid': a port of type '%s' has a list of VLAN ids, given as 'vids'", interfaceName, getPortTypeName(port.type));
    if (!isUntaggedPort(port.type) && !hasVids)
        throw cRuntimeError(this, "The ports entry for interface '%s' has no 'vids': a port of type '%s' needs a list of VLAN ids", interfaceName, getPortTypeName(port.type));
    return port;
}

void Ieee8021adPolicy::checkPort(const char *interfaceName, const Port& port) const
{
    bool requiresSTag = port.type == PORT_TYPE_CNP || port.type == PORT_TYPE_PNP;
    if (requiresSTag != (vlanTagType == VLAN_TAG_TYPE_S))
        throw cRuntimeError(this, "Port type '%s' of interface '%s' requires vlanTagType \"%s\"", getPortTypeName(port.type), interfaceName, requiresSTag ? "s" : "c");
    if (port.vlanIds.empty())
        throw cRuntimeError(this, "Port of interface '%s' has no VLAN id", interfaceName);
    if (isUntaggedPort(port.type) && port.vlanIds.size() != 1)
        throw cRuntimeError(this, "Port of type '%s' of interface '%s' has more than one VLAN id", getPortTypeName(port.type), interfaceName);
    for (int vlanId : port.vlanIds)
        if (vlanId < 1 || vlanId > 4094)
            throw cRuntimeError(this, "Invalid VLAN id %d for interface '%s': expected 1 to 4094", vlanId, interfaceName);
}

void Ieee8021adPolicy::checkInterfaces() const
{
    for (const auto& [interfaceName, port] : ports)
        if (interfaceName != "*" && interfaceTable->findInterfaceByName(interfaceName.c_str()) == nullptr)
            throw cRuntimeError(this, "Interface '%s' of the ports parameter is not in the interface table", interfaceName.c_str());
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto networkInterface = interfaceTable->getInterface(i);
        // the ports of the relay, see RelayInterfaceSelector::isForwardingInterface()
        if (!networkInterface->isLoopback() && networkInterface->isBroadcast() && findInterfaceEntry(ports, networkInterface->getInterfaceName()) == nullptr)
            throw cRuntimeError(this, "Interface '%s' has no entry in the ports parameter, and there is no '*' entry", networkInterface->getInterfaceName());
    }
}

const Ieee8021adPolicy::Port& Ieee8021adPolicy::getPort(const NetworkInterface *networkInterface) const
{
    auto port = findInterfaceEntry(ports, networkInterface->getInterfaceName());
    if (port == nullptr)
        throw cRuntimeError("Interface '%s' has no entry in the ports parameter, and there is no '*' entry", networkInterface->getInterfaceName());
    return *port;
}

void Ieee8021adPolicy::setPort(const NetworkInterface *networkInterface, PortType type, const std::set<int>& vlanIds)
{
    Enter_Method("setPort");
    auto interfaceName = networkInterface->getInterfaceName();
    if (interfaceTable->getInterfaceById(networkInterface->getInterfaceId()) != networkInterface)
        throw cRuntimeError("Interface '%s' is not in the interface table of this module", interfaceName);
    Port port;
    port.type = type;
    port.vlanIds = vlanIds;
    checkPort(interfaceName, port);
    EV_INFO << "Setting port" << EV_FIELD(interfaceName) << EV_FIELD(port) << EV_ENDL;
    ports[interfaceName] = port;
}

void Ieee8021adPolicy::removePort(const NetworkInterface *networkInterface)
{
    Enter_Method("removePort");
    auto interfaceName = networkInterface->getInterfaceName();
    EV_INFO << "Removing port" << EV_FIELD(interfaceName) << EV_ENDL;
    ports.erase(interfaceName);
}

void Ieee8021adPolicy::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == interfaceDeletedSignal)
        ports.erase(check_and_cast<NetworkInterface *>(object)->getInterfaceName());
    else
        throw cRuntimeError("Unknown signal");
}

void Ieee8021adPolicy::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);
    if (gate == upperLayerInGate)
        processSentPacket(packet);
    else if (gate == lowerLayerInGate)
        processReceivedPacket(packet);
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8021adPolicy::processReceivedPacket(Packet *packet)
{
    auto interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceId);
    if (networkInterface == nullptr)
        throw cRuntimeError("Interface %d of the received packet is not in the interface table", interfaceId);
    auto interfaceName = networkInterface->getInterfaceName();
    const auto& port = getPort(networkInterface);
    auto vlanInd = findVlanInd(packet, vlanTagType);
    int vlanId = vlanInd != nullptr ? vlanInd->getVlanId() : -1;
    if (isUntaggedPort(port.type)) {
        // IEEE 802.1Q-2018 6.9.1 b 4 and f: Admit Only Untagged and Priority-tagged frames, classified into the PVID
        if (vlanId != -1 && vlanId != 0) {
            dropNotAcceptedPacket(packet, "the port accepts untagged and priority-tagged frames only");
            return;
        }
        int portVlanId = *port.vlanIds.begin();
        EV_INFO << "Classifying received frame into the VLAN of the port" << EV_FIELD(interfaceName) << EV_FIELD(vlanId) << EV_FIELD(portVlanId) << EV_FIELD(packet) << EV_ENDL;
        addVlanIndIfAbsent(packet, vlanTagType)->setVlanId(portVlanId);
    }
    else {
        // IEEE 802.1Q-2018 6.9.1 a and b 3, 8.6.2: Admit Only VLAN-tagged frames, ingress filtering
        if (port.vlanIds.find(vlanId) == port.vlanIds.end()) {
            dropNotAcceptedPacket(packet, "the frame is not tagged with a VLAN of the port");
            return;
        }
    }
    pushOrSendPacket(packet, upperLayerOutGate, upperLayerSink);
}

void Ieee8021adPolicy::processSentPacket(Packet *packet)
{
    auto interfaceId = packet->getTag<InterfaceReq>()->getInterfaceId();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceId);
    if (networkInterface == nullptr)
        throw cRuntimeError("Interface %d of the sent packet is not in the interface table", interfaceId);
    auto interfaceName = networkInterface->getInterfaceName();
    const auto& port = getPort(networkInterface);
    auto vlanReq = findVlanReq(packet, vlanTagType);
    int vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : -1;
    if (isUntaggedPort(port.type)) {
        // IEEE 802.1Q-2018 8.6.4 and 8.8.2: the member set and the untagged set are the VLAN of the port;
        // a frame without a VLAN comes from the bridge itself and is sent untagged too
        if (vlanId != -1 && port.vlanIds.find(vlanId) == port.vlanIds.end()) {
            dropNotAcceptedPacket(packet, "the frame is not in the VLAN of the port");
            return;
        }
        EV_INFO << "Sending frame untagged" << EV_FIELD(interfaceName) << EV_FIELD(vlanId) << EV_FIELD(packet) << EV_ENDL;
        removeVlanReqIfPresent(packet, vlanTagType);
        removePcpReqIfPresent(packet, vlanTagType);
        removeDropEligibleReqIfPresent(packet, vlanTagType);
        ensureEncapsulationProtocolReq(packet, vlanTagProtocol, false);
    }
    else {
        // IEEE 802.1Q-2018 8.6.4 and 8.8.2: the member set is the VLANs of the port, the untagged set is empty
        if (port.vlanIds.find(vlanId) == port.vlanIds.end()) {
            dropNotAcceptedPacket(packet, vlanId == -1 ? "the frame has no VLAN" : "the frame is not in a VLAN of the port");
            return;
        }
        ensureEncapsulationProtocolReq(packet, vlanTagProtocol, true);
    }
    setDispatchProtocol(packet);
    pushOrSendPacket(packet, lowerLayerOutGate, lowerLayerSink);
}

void Ieee8021adPolicy::dropNotAcceptedPacket(Packet *packet, const char *reason)
{
    EV_WARN << "Port does not accept the frame, dropping packet" << EV_FIELD(reason) << EV_FIELD(packet) << EV_ENDL;
    numDroppedPackets++;
    dropPacket(packet, OTHER_PACKET_DROP);
}

} // namespace inet
