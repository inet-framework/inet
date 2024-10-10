//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ethernet/basic/EthernetEncapsulation.h"

#include "inet/common/INETUtils.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetCommand_m.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(EthernetEncapsulation);

simsignal_t EthernetEncapsulation::encapPkSignal = registerSignal("encapPk");
simsignal_t EthernetEncapsulation::decapPkSignal = registerSignal("decapPk");
simsignal_t EthernetEncapsulation::pauseSentSignal = registerSignal("pauseSent");

std::ostream& operator<<(std::ostream& o, const EthernetEncapsulation::Socket& t)
{
    o << "(id:" << t.socketId
      << ",interfaceId:" << t.interfaceId
      << ",local:" << t.localAddress
      << ",remote:" << t.remoteAddress
      << ",protocol" << (t.protocol ? t.protocol->getName() : "<null>")
      << ",steal:" << (t.steal ? "on" : "off")
      << ")";
    return o;
}

EthernetEncapsulation::~EthernetEncapsulation()
{
    for (auto it : socketIdToSocketMap)
        delete it.second;
}

bool EthernetEncapsulation::Socket::matches(Packet *packet, int ifaceId, const Ptr<const EthernetMacHeader>& ethernetMacHeader)
{
    if (interfaceId != -1 && interfaceId != ifaceId)
        return false;
    if (!remoteAddress.isUnspecified() && !ethernetMacHeader->getSrc().isBroadcast() && ethernetMacHeader->getSrc() != remoteAddress)
        return false;
    if (!localAddress.isUnspecified() && !ethernetMacHeader->getDest().isBroadcast() && ethernetMacHeader->getDest() != localAddress)
        return false;
    if (protocol != nullptr && packet->getTag<PacketProtocolTag>()->getProtocol() != protocol)
        return false;
    return true;
}

void EthernetEncapsulation::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        fcsMode = parseFcsMode(par("fcsMode"));
        seqNum = 0;
        WATCH(seqNum);
        totalFromHigherLayer = totalFromMAC = totalPauseSent = 0;
        interfaceTable.reference(this, "interfaceTableModule", true);
        lowerLayerSink.reference(gate("lowerLayerOut"), true);
        upperLayerSink.reference(gate("upperLayerOut"), true);

        WATCH_PTRSET(upperProtocols);
        WATCH_PTRMAP(socketIdToSocketMap);
        WATCH(totalFromHigherLayer);
        WATCH(totalFromMAC);
        WATCH(totalPauseSent);
    }
}

void EthernetEncapsulation::handleMessageWhenUp(cMessage *msg)
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

void EthernetEncapsulation::processCommandFromHigherLayer(Request *msg)
{
    msg->removeTagIfPresent<DispatchProtocolReq>();
    auto ctrl = msg->getControlInfo();
    if (dynamic_cast<Ieee802PauseCommand *>(ctrl) != nullptr)
        handleSendPause(msg);
    else if (auto bindCommand = dynamic_cast<EthernetBindCommand *>(ctrl)) {
        int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
        Socket *socket = new Socket(socketId);
        socket->interfaceId = msg->getTag<InterfaceReq>()->getInterfaceId();
        socket->localAddress = bindCommand->getLocalAddress();
        socket->remoteAddress = bindCommand->getRemoteAddress();
        socket->protocol = bindCommand->getProtocol();
        socket->steal = bindCommand->getSteal();
        socketIdToSocketMap[socketId] = socket;
        delete msg;
    }
    else if (dynamic_cast<SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketMap.find(socketId);
        if (it != socketIdToSocketMap.end()) {
            delete it->second;
            socketIdToSocketMap.erase(it);
            auto indication = new Indication("closed", SOCKET_I_CLOSED);
            auto ctrl = new SocketClosedIndication();
            indication->setControlInfo(ctrl);
            indication->addTag<SocketInd>()->setSocketId(socketId);
            send(indication, "transportOut");
            delete msg;
        }
    }
    else if (dynamic_cast<SocketDestroyCommand *>(ctrl) != nullptr) {
        int socketId = check_and_cast<Request *>(msg)->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketMap.find(socketId);
        if (it != socketIdToSocketMap.end()) {
            delete it->second;
            socketIdToSocketMap.erase(it);
            delete msg;
        }
    }
    else
        throw cRuntimeError("Unknown command: '%s' with %s", msg->getName(), ctrl->getClassName());
}

void EthernetEncapsulation::refreshDisplay() const
{
    OperationalBase::refreshDisplay();
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalFromMAC, totalFromHigherLayer);
    getDisplayString().setTagArg("t", 0, buf);
}

void EthernetEncapsulation::processPacketFromHigherLayer(Packet *packet)
{
    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV_INFO << "Received packet from higher layer" << EV_FIELD(packet) << EV_ENDL;

    if (packet->getDataLength() > MAX_ETHERNET_DATA_BYTES)
        throw cRuntimeError("packet length from higher layer (%s) exceeds maximum Ethernet payload length (%s)", packet->getDataLength().str().c_str(), MAX_ETHERNET_DATA_BYTES.str().c_str());

    totalFromHigherLayer++;
    emit(encapPkSignal, packet);

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    int typeOrLength = -1;
    const auto& protocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    const Protocol *protocol = protocolTag->getProtocol();
    if (protocol && *protocol != Protocol::ieee8022llc)
        typeOrLength = ProtocolGroup::getEthertypeProtocolGroup()->getProtocolNumber(protocol);
    else
        typeOrLength = packet->getByteLength();

    const auto& ethHeader = makeShared<EthernetMacHeader>();
    auto macAddressReq = packet->getTag<MacAddressReq>();
    auto srcAddr = macAddressReq->getSrcAddress();
    auto interfaceReq = packet->getTag<InterfaceReq>();
    auto networkInterface = interfaceTable->getInterfaceById(interfaceReq->getInterfaceId());
    if (srcAddr.isUnspecified() && networkInterface != nullptr)
        srcAddr = networkInterface->getMacAddress();
    ethHeader->setSrc(srcAddr);
    ethHeader->setDest(macAddressReq->getDestAddress());
    ethHeader->setTypeOrLength(typeOrLength);
    packet->insertAtFront(ethHeader);
    const auto& ethernetFcs = makeShared<EthernetFcs>();
    ethernetFcs->setFcsMode(fcsMode);
    packet->insertAtBack(ethernetFcs);
    protocolTag->setProtocol(&Protocol::ethernetMac);
    packet->removeTagIfPresent<DispatchProtocolReq>();
    EV_INFO << "Sending packet to lower layer" << EV_FIELD(packet) << EV_ENDL;
    lowerLayerSink.pushPacket(packet);
}

void EthernetEncapsulation::processPacketFromMac(Packet *packet)
{
    const Protocol *payloadProtocol = nullptr;

    EV_DETAIL << "Received packet from lower layer" << EV_FIELD(packet) << EV_ENDL;

    int iface = packet->getTag<InterfaceInd>()->getInterfaceId();
    auto ethHeader = packet->popAtFront<EthernetMacHeader>();
    packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);

    // add MacAddressInd to packet
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(ethHeader->getSrc());
    macAddressInd->setDestAddress(ethHeader->getDest());

    // remove Padding if possible
    if (isIeee8023Header(*ethHeader)) {
        b payloadLength = B(ethHeader->getTypeOrLength());
        if (packet->getDataLength() < payloadLength)
            throw cRuntimeError("incorrect payload length in ethernet frame");      // TODO alternative: drop packet
        packet->setBackOffset(packet->getFrontOffset() + payloadLength);
        payloadProtocol = &Protocol::ieee8022llc;
    }
    else if (isEth2Header(*ethHeader)) {
        payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(ethHeader->getTypeOrLength());
    }
    else
        throw cRuntimeError("Unknown ethernet header");

    if (payloadProtocol != nullptr) {
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    }
    else {
        packet->removeTagIfPresent<PacketProtocolTag>();
        packet->removeTagIfPresent<DispatchProtocolReq>();
    }
    bool steal = false;
    for (auto it : socketIdToSocketMap) {
        auto socket = it.second;
        if (socket->matches(packet, iface, ethHeader)) {
            auto packetCopy = packet->dup();
            packetCopy->setKind(SOCKET_I_DATA);
            packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(it.first);
            EV_INFO << "Sending packet to socket" << EV_FIELD(socketId, it.first) << EV_FIELD(packet, packetCopy) << EV_ENDL;
            upperLayerSink.pushPacket(packetCopy);
            steal |= socket->steal;
        }
    }
    if (steal)
        delete packet;
    else if (hasUpperProtocol(payloadProtocol)) {
        totalFromMAC++;
        emit(decapPkSignal, packet);

        // pass up to higher layers.
        EV_INFO << "Sending packet to upper layer" << EV_FIELD(packet) << EV_ENDL;
        upperLayerSink.pushPacket(packet);
    }
    else {
        EV_WARN << "Unknown protocol, dropping packet\n";
        PacketDropDetails details;
        details.setReason(NO_PROTOCOL_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void EthernetEncapsulation::handleSendPause(cMessage *msg)
{
    Ieee802PauseCommand *etherctrl = dynamic_cast<Ieee802PauseCommand *>(msg->getControlInfo());
    if (!etherctrl)
        throw cRuntimeError("PAUSE command `%s' from higher layer received without Ieee802PauseCommand controlinfo", msg->getName());
    MacAddress dest = etherctrl->getDestinationAddress();
    int pauseUnits = etherctrl->getPauseUnits();
    delete msg;

    EV_DETAIL << "Creating and sending PAUSE frame, with duration = " << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[40];
    sprintf(framename, "pause-%d-%d", getId(), seqNum++);
    auto packet = new Packet(framename);
    const auto& frame = makeShared<EthernetPauseFrame>();
    const auto& hdr = makeShared<EthernetMacHeader>();
    frame->setPauseTime(pauseUnits);
    if (dest.isUnspecified())
        dest = MacAddress::MULTICAST_PAUSE_ADDRESS;
    hdr->setDest(dest);
    packet->insertAtFront(frame);
    hdr->setTypeOrLength(ETHERTYPE_FLOW_CONTROL);
    packet->insertAtFront(hdr);
    const auto& ethernetFcs = makeShared<EthernetFcs>();
    ethernetFcs->setFcsMode(fcsMode);
    packet->insertAtBack(ethernetFcs);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);

    EV_INFO << "Sending " << frame << " to lower layer.\n";
    send(packet, "lowerLayerOut");

    emit(pauseSentSignal, pauseUnits);
    totalPauseSent++;
}

bool EthernetEncapsulation::hasUpperProtocol(const Protocol *protocol)
{
    if (protocol == nullptr)
        return false;
    else if (contains(upperProtocols, protocol))
        return true;
    else {
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(protocol);
        dispatchProtocolReq.setServicePrimitive(SP_INDICATION);
        if (findModuleInterface(gate("upperLayerOut"), typeid(IPassivePacketSink), &dispatchProtocolReq) == nullptr)
            return false;
        else {
            upperProtocols.insert(protocol);
            return true;
        }
    }
}

void EthernetEncapsulation::clearSockets()
{
    for (auto& elem : socketIdToSocketMap) {
        delete elem.second;
        elem.second = nullptr;
    }
    socketIdToSocketMap.clear();
}

void EthernetEncapsulation::handleStartOperation(LifecycleOperation *operation)
{
    clearSockets();
}

void EthernetEncapsulation::handleStopOperation(LifecycleOperation *operation)
{
    clearSockets();
}

void EthernetEncapsulation::handleCrashOperation(LifecycleOperation *operation)
{
    clearSockets();
}

void EthernetEncapsulation::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (gate->isName("upperLayerIn"))
        processPacketFromHigherLayer(packet);
    else
        processPacketFromMac(packet);
}

void EthernetEncapsulation::bind(int socketId, int interfaceId, const MacAddress &localAddress, const MacAddress &remoteAddress, const Protocol *protocol, bool steal)
{
    Enter_Method("bind");
    Socket *socket = new Socket(socketId);
    socket->interfaceId = interfaceId;
    socket->localAddress = localAddress;
    socket->remoteAddress = remoteAddress;
    socket->protocol = protocol;
    socket->steal = steal;
    socketIdToSocketMap[socketId] = socket;
    EV_INFO << "Socket bound" << EV_FIELD(socketId) << EV_FIELD(interfaceId) << EV_FIELD(localAddress) << EV_FIELD(remoteAddress) << EV_FIELD(protocol) << EV_FIELD(steal) << EV_ENDL;
}

} // namespace inet

