//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/llc/Ieee80211LlcEpd.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211LlcEpd);

void Ieee80211LlcEpd::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        lowerLayerSink.reference(gate("lowerLayerOut"), true);
        upperLayerSink.reference(gate("upperLayerOut"), true);
    }
}

void Ieee80211LlcEpd::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        encapsulate(packet);
        lowerLayerSink.pushPacket(packet);
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        decapsulate(packet);
        if (packet->getTag<PacketProtocolTag>()->getProtocol() != nullptr)
            upperLayerSink.pushPacket(packet);
        else {
            EV_WARN << "Unknown protocol, dropping packet\n";
            PacketDropDetails details;
            details.setReason(NO_PROTOCOL_FOUND);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    else
        throw cRuntimeError("Unknown message");
}

void Ieee80211LlcEpd::encapsulate(Packet *frame)
{
    const Protocol *protocol = frame->getTag<PacketProtocolTag>()->getProtocol();
    int ethType = ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol);
    if (ethType == -1)
        throw cRuntimeError("EtherType not found for protocol %s", protocol ? protocol->getName() : "(nullptr)");
    const auto& llcHeader = makeShared<Ieee802EpdHeader>();
    llcHeader->setEtherType(ethType);
    frame->insertAtFront(llcHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee802epd);
}

void Ieee80211LlcEpd::decapsulate(Packet *frame)
{
    const auto& epdHeader = frame->popAtFront<Ieee802EpdHeader>();
    auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(epdHeader->getEtherType());
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

const Protocol *Ieee80211LlcEpd::getProtocol() const
{
    return &Protocol::ieee802epd;
}

void Ieee80211LlcEpd::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), gate->getId());
    handleMessage(packet);
}

} // namespace ieee80211
} // namespace inet

