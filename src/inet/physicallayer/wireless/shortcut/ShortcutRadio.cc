//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/shortcut/ShortcutRadio.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/wireless/shortcut/ShortcutPhyHeader_m.h"

namespace inet {
namespace physicallayer {

Define_Module(ShortcutRadio);

void ShortcutRadio::initialize(int stage)
{
    PhysicalLayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        bitrate = par("bitrate");
        lengthOverhead = &par("lengthOverhead");
        durationOverhead = &par("durationOverhead");
        propagationDelay = &par("propagationDelay");
        packetLoss = &par("packetLoss");
        gate("radioIn")->setDeliverImmediately(true);
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_LINK_LAYER) {
        auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto networkInterface = CHK(interfaceTable->findInterfaceByInterfaceModule(this));
        auto address = networkInterface->getMacAddress();
        shortcutRadios[address] = this;
    }
}

void ShortcutRadio::handleMessageWhenUp(cMessage *message)
{
    if (message->getArrivalGate() == gate("radioIn"))
        receiveFromPeer(check_and_cast<Packet *>(message));
    else
        PhysicalLayerBase::handleMessageWhenUp(message);
}

void ShortcutRadio::handleUpperPacket(Packet *packet)
{
    auto destination = packet->getTag<MacAddressReq>()->getDestAddress();
    transmissionState = IRadio::TRANSMISSION_STATE_TRANSMITTING;
    emit(transmissionStateChangedSignal, transmissionState);
    if (destination.isBroadcast()) {
        for (auto it : shortcutRadios)
            if (it.second != this)
                sendToPeer(packet->dup(), it.second);
        delete packet;
    }
    else {
        auto peer = findPeer(destination);
        if (peer != nullptr)
            sendToPeer(packet, peer);
        else
            throw cRuntimeError("ShortcutRadio not found");
    }
    transmissionState = IRadio::TRANSMISSION_STATE_IDLE; // TODO zero time transmission simulated
    emit(transmissionStateChangedSignal, transmissionState);
}

ShortcutRadio *ShortcutRadio::findPeer(MacAddress address)
{
    auto it = shortcutRadios.find(address);
    return (it == shortcutRadios.end()) ? nullptr : it->second;
}

void ShortcutRadio::sendToPeer(Packet *packet, ShortcutRadio *peer)
{
    if (dblrand() < packetLoss->doubleValue()) {
        EV_WARN << "Packet " << packet << " lost.";
        delete packet;
    }
    else {
        auto length = b(lengthOverhead->intValue());
        if (length < b(0))
            throw cRuntimeError("invalid lengthOverhead value: %" PRId64 " bit", length.get());
        if (length > b(0)) {
            auto& protocolTag = packet->getTagForUpdate<PacketProtocolTag>();
            auto header = makeShared<ShortcutPhyHeader>();
            header->setChunkLength(length);
            header->setPayloadProtocol(protocolTag->getProtocol());
            packet->insertAtFront(header);
            protocolTag->setProtocol(&Protocol::shortcutPhy);
        }
        simtime_t transmissionDuration = packet->getBitLength() / bitrate + durationOverhead->doubleValue();
        packet->setDuration(transmissionDuration);
        sendDirect(packet, propagationDelay->doubleValue(), transmissionDuration, peer->gate("radioIn")->getPathStartGate());
    }
}

void ShortcutRadio::receiveFromPeer(Packet *packet)
{
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    if (packetProtocolTag->getProtocol() == &Protocol::shortcutPhy) {
        const auto& header = packet->popAtFront<ShortcutPhyHeader>();
        packetProtocolTag->setProtocol(header->getPayloadProtocol());
    }
    emit(packetSentToUpperSignal, packet);
    send(packet, gate("upperLayerOut"));
}

} // namespace physicallayer
} // namespace inet

