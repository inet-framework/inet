//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211LlcEpd.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeader_m.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211LlcEpd);

void Ieee80211LlcEpd::initialize(int stage)
{
}

void Ieee80211LlcEpd::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        encapsulate(packet);
        send(packet, "lowerLayerOut");
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        decapsulate(packet);
        if (packet->getTag<PacketProtocolTag>()->getProtocol() != nullptr)
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
        throw cRuntimeError("Unknown message");
}

void Ieee80211LlcEpd::encapsulate(Packet *frame)
{
    const Protocol* protocol = frame->getTag<PacketProtocolTag>()->getProtocol();
    int ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
    if (ethType == -1)
        throw cRuntimeError("EtherType not found for protocol %s", protocol ? protocol->getName() : "(nullptr)");
    const auto& llcHeader = makeShared<Ieee80211EtherTypeHeader>();
    llcHeader->setEtherType(ethType);
    frame->insertAtFront(llcHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee80211EtherType);
    frame->addTagIfAbsent<LlcProtocolTag>()->setProtocol(&Protocol::ieee80211EtherType);
}

void Ieee80211LlcEpd::decapsulate(Packet *frame)
{
    const auto& llcHeader = frame->popAtFront<Ieee80211EtherTypeHeader>();
    const Protocol *payloadProtocol = llcHeader->getProtocol();
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

const Protocol *Ieee80211LlcEpd::getProtocol() const
{
    return &Protocol::ieee80211EtherType;
}

} // namespace ieee80211
} // namespace inet

