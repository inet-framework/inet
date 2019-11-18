//
// Copyright (C) OpenSim Ltd.
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/emulation/networklayer/ipv4/Ipv4Encap.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"

namespace inet {

Define_Module(Ipv4Encap);

void Ipv4Encap::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        defaultTimeToLive = 16; // par("timeToLive");
        defaultMCTimeToLive = 16; // par("multicastTimeToLive");
        crcMode = CRC_COMPUTED;
        registerService(Protocol::ipv4, gate("upperLayerIn"), nullptr);
        registerProtocol(Protocol::ipv4, nullptr, gate("upperLayerOut"));
    }
}

void Ipv4Encap::handleMessage(cMessage *msg)
{
    if (auto request = dynamic_cast<Request *>(msg))
        handleRequest(request);
    else if (msg->getArrivalGate()->isName("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(msg);
        EV << "Encapsulating\n";
        encapsulate(packet);
        EV << "SEnding\n";
        send(packet, "lowerLayerOut");
    }
    else {
        auto packet = check_and_cast<Packet *>(msg);
        auto ipv4HeaderPosition = packet->getFrontOffset();
        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
        const Protocol *protocol = ipv4Header->getProtocol();
        auto remoteAddress(ipv4Header->getSrcAddress());
        auto localAddress(ipv4Header->getDestAddress());
        EV << "Receiving\n";
        decapsulate(packet);
        bool hasSocket = false;
        for (const auto &elem: socketIdToSocketDescriptor) {
            if (elem.second->protocolId == protocol->getId() &&
                (elem.second->localAddress.isUnspecified() || elem.second->localAddress == localAddress) &&
                (elem.second->remoteAddress.isUnspecified() || elem.second->remoteAddress == remoteAddress))
            {
                auto *packetCopy = packet->dup();
                packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(elem.second->socketId);
                EV_INFO << "Passing up to socket " << elem.second->socketId << "\n";
//                emit(packetSentToUpperSignal, packetCopy);
                send(packetCopy, "upperLayerOut");
                hasSocket = true;
            }
        }
        if (upperProtocols.find(protocol) != upperProtocols.end()) {
            EV_INFO << "Passing up to protocol " << protocol << "\n";
//            emit(packetSentToUpperSignal, packet);
            send(packet, "upperLayerOut");
        }
        else if (hasSocket) {
            delete packet;
        }
        else {
            EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
            packet->setFrontOffset(ipv4HeaderPosition);
//            const InterfaceEntry* fromIE = getSourceInterface(packet);
//            sendIcmpError(packet, fromIE ? fromIE->getInterfaceId() : -1, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
        }
        send(packet, "upperLayerOut");
    }
}

void Ipv4Encap::handleRequest(Request *request)
{
    auto ctrl = request->getControlInfo();
    if (ctrl == nullptr)
        throw cRuntimeError("Request '%s' arrived without controlinfo", request->getName());
    else if (Ipv4SocketBindCommand *command = dynamic_cast<Ipv4SocketBindCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocol()->getId(), command->getLocalAddress());
        socketIdToSocketDescriptor[socketId] = descriptor;
        delete request;
    }
    else if (Ipv4SocketConnectCommand *command = dynamic_cast<Ipv4SocketConnectCommand *>(ctrl)) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        if (socketIdToSocketDescriptor.find(socketId) == socketIdToSocketDescriptor.end())
            throw cRuntimeError("Ipv4Socket: should use bind() before connect()");
        socketIdToSocketDescriptor[socketId]->remoteAddress = command->getRemoteAddress();
        delete request;
    }
    else if (dynamic_cast<Ipv4SocketCloseCommand *>(ctrl) != nullptr) {
        int socketId = 0; request->getTag<SocketReq>()->getSocketId();
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

void Ipv4Encap::encapsulate(Packet *transportPacket)
{
    const auto& ipv4Header = makeShared<Ipv4Header>();

    auto l3AddressReq = transportPacket->removeTag<L3AddressReq>();
    Ipv4Address src = l3AddressReq->getSrcAddress().toIpv4();
    Ipv4Address dest = l3AddressReq->getDestAddress().toIpv4();
    delete l3AddressReq;

    ipv4Header->setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(transportPacket->getTag<PacketProtocolTag>()->getProtocol()));

    auto hopLimitReq = transportPacket->removeTagIfPresent<HopLimitReq>();
    short ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    delete hopLimitReq;
    bool dontFragment = false;
    if (auto dontFragmentReq = transportPacket->removeTagIfPresent<FragmentationReq>()) {
        dontFragment = dontFragmentReq->getDontFragment();
        delete dontFragmentReq;
    }

    // set source and destination address
    ipv4Header->setDestAddress(dest);

    // when source address was given, use it; otherwise it'll get the address
    // of the outgoing interface after routing
    if (!src.isUnspecified())
        ipv4Header->setSrcAddress(src);

    // set other fields
    if (TosReq *tosReq = transportPacket->removeTagIfPresent<TosReq>()) {
        ipv4Header->setTypeOfService(tosReq->getTos());
        delete tosReq;
        if (transportPacket->findTag<DscpReq>())
            throw cRuntimeError("TosReq and DscpReq found together");
        if (transportPacket->findTag<EcnReq>())
            throw cRuntimeError("TosReq and EcnReq found together");
    }
    if (DscpReq *dscpReq = transportPacket->removeTagIfPresent<DscpReq>()) {
        ipv4Header->setDscp(dscpReq->getDifferentiatedServicesCodePoint());
        delete dscpReq;
    }
    if (EcnReq *ecnReq = transportPacket->removeTagIfPresent<EcnReq>()) {
        ipv4Header->setEcn(ecnReq->getExplicitCongestionNotification());
        delete ecnReq;
    }

    ipv4Header->setMoreFragments(false);
    ipv4Header->setDontFragment(dontFragment);
    ipv4Header->setFragmentOffset(0);

    if (ttl != -1) {
        ASSERT(ttl > 0);
    }
    else if (ipv4Header->getDestAddress().isLinkLocalMulticast())
        ttl = 1;
    else if (ipv4Header->getDestAddress().isMulticast())
        ttl = defaultMCTimeToLive;
    else
        ttl = defaultTimeToLive;
    ipv4Header->setTimeToLive(ttl);
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + transportPacket->getDataLength());
    ipv4Header->setCrcMode(crcMode);
    ipv4Header->setCrc(0);
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            ipv4Header->setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            MemoryOutputStream ipv4HeaderStream;
            Chunk::serialize(ipv4HeaderStream, ipv4Header);
            // compute the CRC
            uint16_t crc = TcpIpChecksum::checksum(ipv4HeaderStream.getData());
            ipv4Header->setCrc(crc);
            // crc will be calculated in fragmentAndSend()
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
    insertNetworkProtocolHeader(transportPacket, Protocol::ipv4, ipv4Header);
    // setting Ipv4 options is currently not supported
}

void Ipv4Encap::decapsulate(Packet *packet)
{
    // decapsulate transport packet
    const auto& ipv4Header = packet->popAtFront<Ipv4Header>();

    // create and fill in control info
    packet->addTagIfAbsent<DscpInd>()->setDifferentiatedServicesCodePoint(ipv4Header->getDscp());
    packet->addTagIfAbsent<EcnInd>()->setExplicitCongestionNotification(ipv4Header->getEcn());
    packet->addTagIfAbsent<TosInd>()->setTos(ipv4Header->getTypeOfService());

    // original Ipv4 datagram might be needed in upper layers to send back ICMP error message

    auto transportProtocol = ProtocolGroup::ipprotocol.getProtocol(ipv4Header->getProtocolId());
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(transportProtocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(transportProtocol);
    auto l3AddressInd = packet->addTagIfAbsent<L3AddressInd>();
    l3AddressInd->setSrcAddress(ipv4Header->getSrcAddress());
    l3AddressInd->setDestAddress(ipv4Header->getDestAddress());
    packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(ipv4Header->getTimeToLive());
}

} // namespace inet

