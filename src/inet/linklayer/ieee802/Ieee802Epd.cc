//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802/Ieee802Epd.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {

Define_Module(Ieee802Epd);

void Ieee802Epd::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        // register service and protocol
        registerService(Protocol::ieee802epd, gate("upperLayerIn"), gate("upperLayerOut"));
        registerProtocol(Protocol::ieee802epd, gate("lowerLayerOut"), gate("lowerLayerIn"));
    }
}

void Ieee802Epd::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        encapsulate(packet);
        send(packet, "lowerLayerOut");
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        decapsulate(packet);
        if (packet->findTag<PacketProtocolTag>())
            send(packet, "upperLayerOut");
        else {
            EV_WARN << "Unknown protocol, dropping packet\n";
            PacketDropDetails details;
            details.setReason(NO_PROTOCOL_FOUND);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    else
        throw cRuntimeError("Message arrived on unknown gate '%s'", message->getArrivalGate()->getName());
}

void Ieee802Epd::encapsulate(Packet *frame)
{
    const Protocol *protocol = frame->getTag<PacketProtocolTag>()->getProtocol();
    int ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
    if (ethType == -1)
        throw cRuntimeError("EtherType not found for protocol %s", protocol ? protocol->getName() : "(nullptr)");
    const auto& llcHeader = makeShared<Ieee802EpdHeader>();
    llcHeader->setEtherType(ethType);
    frame->insertAtFront(llcHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee802epd);
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
}

void Ieee802Epd::decapsulate(Packet *frame)
{
    const auto& epdHeader = frame->popAtFront<Ieee802EpdHeader>();
    auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(epdHeader->getEtherType());
    if (payloadProtocol) {
        frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
        frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    }
    else {
        frame->removeTagIfPresent<DispatchProtocolReq>();
        frame->removeTagIfPresent<PacketProtocolTag>();
    }
}

} // namespace inet

