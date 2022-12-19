//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

Define_Module(Ieee8022Llc);

Ieee8022Llc::~Ieee8022Llc()
{
    clearSockets();
}

void Ieee8022Llc::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // TODO parameterization for llc or snap?
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        if (par("registerProtocol").boolValue()) { // FIXME //KUDGE should redesign place of EthernetEncapsulation and LLC modules
            // register service and protocol
            registerService(Protocol::ieee8022llc, gate("upperLayerIn"), gate("upperLayerOut"));
            registerProtocol(Protocol::ieee8022llc, gate("lowerLayerOut"), gate("lowerLayerIn"));
        }

        WATCH_PTRMAP(socketIdToSocketDescriptor);
        WATCH_PTRSET(upperProtocols);
    }
}

void Ieee8022Llc::handleMessageWhenUp(cMessage *msg)
{
    if (msg->arrivedOn("upperLayerIn")) {
        // from upper layer
        EV_INFO << "Received " << msg << " from upper layer." << endl;
        if (msg->isPacket())
            processPacketFromHigherLayer(check_and_cast<Packet *>(msg));
        else
            processCommandFromHigherLayer(check_and_cast<Request *>(msg));
    }
    else if (msg->arrivedOn("lowerLayerIn")) {
        EV_INFO << "Received " << msg << " from lower layer." << endl;
        processPacketFromMac(check_and_cast<Packet *>(msg));
    }
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8022Llc::processPacketFromHigherLayer(Packet *packet)
{
    encapsulate(packet);
    send(packet, "lowerLayerOut");
}

void Ieee8022Llc::processCommandFromHigherLayer(Request *request)
{
    auto ctrl = request->getControlInfo();
    if (ctrl == nullptr)
        throw cRuntimeError("Request '%s' arrived without controlinfo", request->getName());
    else if (auto command = dynamic_cast<Ieee8022LlcSocketOpenCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getLocalSap());
        socketIdToSocketDescriptor[socketId] = descriptor;
        delete request;
    }
    else if (dynamic_cast<SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete request;
        auto indication = new Indication("closed", SOCKET_I_CLOSED);
        auto ctrl = new SocketClosedIndication();
        indication->setControlInfo(ctrl);
        indication->addTag<SocketInd>()->setSocketId(socketId);
        send(indication, "upperLayerOut");

    }
    else if (dynamic_cast<SocketDestroyCommand *>(ctrl) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete request;
    }
    else
        throw cRuntimeError("Unknown command: '%s' with %s", request->getName(), ctrl->getClassName());
}

bool Ieee8022Llc::deliverCopyToSockets(Packet *packet)
{
    bool isSent = false;
    if (auto sap = packet->findTag<Ieee802SapInd>()) {
        int localSap = sap->getDsap();
        int remoteSap = sap->getSsap();
        for (const auto& elem : socketIdToSocketDescriptor) {
            if ((elem.second->localSap == localSap || elem.second->localSap == -1)
                && (elem.second->remoteSap == remoteSap || elem.second->remoteSap == -1))
            {
                auto *packetCopy = packet->dup();
                packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
                EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
                packetCopy->setKind(SOCKET_I_DATA);
                send(packetCopy, "upperLayerOut");
                isSent = true;
            }
        }
    }
    return isSent;
}

bool Ieee8022Llc::isDeliverableToUpperLayer(Packet *packet)
{
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    return (protocolTag != nullptr && contains(upperProtocols, protocolTag->getProtocol()));
}

void Ieee8022Llc::processPacketFromMac(Packet *packet)
{
    decapsulate(packet);
    bool isSent = deliverCopyToSockets(packet);

    if (isDeliverableToUpperLayer(packet)) {
        send(packet, "upperLayerOut");
    }
    else {
        if (!isSent) {
            EV_WARN << "Unknown protocol, dropping packet\n";
            PacketDropDetails details;
            details.setReason(NO_PROTOCOL_FOUND);
            emit(packetDroppedSignal, packet, &details);
        }
        delete packet;
    }
}

void Ieee8022Llc::encapsulate(Packet *frame)
{
    const auto& sapReq = frame->findTag<Ieee802SapReq>();
    const auto& protocolTag = frame->findTag<PacketProtocolTag>();
    const Protocol *protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    int ethType = -1;
    int snapOui = -1;
    if (sapReq == nullptr && protocol != nullptr) {
        ethType = ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol);
        if (ethType == -1)
            snapOui = ProtocolGroup::getSnapOuiProtocolGroup()->findProtocolNumber(protocol);
    }
    if (ethType != -1 || snapOui != -1) {
        const auto& snapHeader = makeShared<Ieee8022LlcSnapHeader>();
        if (ethType != -1) {
            snapHeader->setOui(0);
            snapHeader->setProtocolId(ethType);
        }
        else {
            snapHeader->setOui(snapOui);
            snapHeader->setProtocolId(-1); // FIXME get value from a tag (e.g. protocolTag->getSubId() ???)
        }
        frame->insertAtFront(snapHeader);
    }
    else {
        const auto& llcHeader = makeShared<Ieee8022LlcHeader>();
        int sapData = ProtocolGroup::getIeee8022ProtocolGroup()->findProtocolNumber(protocol);
        if (sapReq == nullptr && sapData != -1) {
            llcHeader->setSsap((sapData >> 8) & 0xFF);
            llcHeader->setDsap(sapData & 0xFF);
            llcHeader->setControl(3);
        }
        else if (sapReq != nullptr) {
            llcHeader->setSsap(sapReq->getSsap());
            llcHeader->setDsap(sapReq->getDsap());
            llcHeader->setControl(3); // TODO get from sapTag
        }
        else
            throw cRuntimeError("Missing the required Ieee802SapReq for LLC header, or protocol number for SNAP header.");
        frame->insertAtFront(llcHeader);
    }
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022llc);
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
}

void Ieee8022Llc::decapsulate(Packet *frame)
{
    const auto& llcHeader = frame->popAtFront<Ieee8022LlcHeader>();

    auto sapInd = frame->addTagIfAbsent<Ieee802SapInd>();
    sapInd->setSsap(llcHeader->getSsap());
    sapInd->setDsap(llcHeader->getDsap());
    // TODO control?

    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
    }
    auto payloadProtocol = getProtocol(llcHeader);
    if (payloadProtocol) {
        frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
        frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    }
    else {
        frame->removeTagIfPresent<DispatchProtocolReq>();
        frame->removeTagIfPresent<PacketProtocolTag>();
    }
}

const Protocol *Ieee8022Llc::getProtocol(const Ptr<const Ieee8022LlcHeader>& llcHeader)
{
    const Protocol *payloadProtocol = nullptr;
    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
        if (snapHeader->getOui() == 0)
            payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(snapHeader->getProtocolId());
        else
            payloadProtocol = ProtocolGroup::getSnapOuiProtocolGroup()->findProtocol(snapHeader->getOui());
    }
    else {
        int32_t sapData = ((llcHeader->getSsap() & 0xFF) << 8) | (llcHeader->getDsap() & 0xFF);
        payloadProtocol = ProtocolGroup::getIeee8022ProtocolGroup()->findProtocol(sapData); // do not use getProtocol
    }
    return payloadProtocol;
}

void Ieee8022Llc::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void Ieee8022Llc::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    // KLUDGE this should be here: if (!strcmp("upperLayerOut", gate->getBaseName()))
    // but then the register protocol calls are lost, because they can't go through the traffic conditioner
    upperProtocols.insert(&protocol);
}

std::ostream& operator<<(std::ostream& o, const Ieee8022Llc::SocketDescriptor& t)
{
    o << "(id:" << t.socketId << ",lsap:" << t.localSap << ",rsap" << t.remoteSap << ")";
    return o;
}

void Ieee8022Llc::clearSockets()
{
    for (auto& elem : socketIdToSocketDescriptor) {
        delete elem.second;
        elem.second = nullptr;
    }
    socketIdToSocketDescriptor.clear();
}

void Ieee8022Llc::handleStartOperation(LifecycleOperation *operation)
{
    clearSockets();
}

void Ieee8022Llc::handleStopOperation(LifecycleOperation *operation)
{
    clearSockets();
}

void Ieee8022Llc::handleCrashOperation(LifecycleOperation *operation)
{
    clearSockets();
}

} // namespace inet

