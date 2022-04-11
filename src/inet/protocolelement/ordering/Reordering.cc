//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/ordering/Reordering.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/ordering/SequenceNumberHeader_m.h"

namespace inet {

Define_Module(Reordering);

Reordering::~Reordering()
{
    for (auto it : packets)
        delete it.second;
}

void Reordering::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::sequenceNumber, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::sequenceNumber, outputGate, nullptr);
    }
}

void Reordering::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto header = packet->popAtFront<SequenceNumberHeader>();
    auto sequenceNumber = header->getSequenceNumber();
    packets[sequenceNumber] = packet;
    if (sequenceNumber == expectedSequenceNumber) {
        while (true) {
            auto it = packets.find(expectedSequenceNumber);
            if (it == packets.end())
                break;
            pushOrSendPacket(it->second, outputGate, consumer);
            packets.erase(it);
            expectedSequenceNumber++;
        }
    }
}

} // namespace inet

