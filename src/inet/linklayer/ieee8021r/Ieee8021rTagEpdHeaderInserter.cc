//
// Copyright (C) 2020 OpenSim Ltd.
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

#include "inet/linklayer/ieee8021r/Ieee8021rTagEpdHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/redundancy/SequenceNumberTag_m.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021rTagEpdHeaderInserter);

void Ieee8021rTagEpdHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *nextProtocolAsString = par("nextProtocol");
        if (*nextProtocolAsString != '\0')
            nextProtocol = Protocol::getProtocol(nextProtocolAsString);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerService(Protocol::ieee8021rTag, inputGate, nullptr);
}

void Ieee8021rTagEpdHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021rTagEpdHeader>();
    auto sequenceNumberReq = packet->removeTagIfPresent<SequenceNumberReq>();
    if (sequenceNumberReq != nullptr) {
        auto sequenceNumber = sequenceNumberReq->getSequenceNumber();
        EV_INFO << "Setting sequence number" << EV_FIELD(sequenceNumber) << EV_ENDL;
        header->setSequenceNumber(sequenceNumber);
        packet->addTagIfAbsent<SequenceNumberInd>()->setSequenceNumber(sequenceNumber);
    }
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::ethertype.findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setProtocol(&Protocol::ieee8021rTag);
    packetProtocolTag->setFrontOffset(b(0));
    const Protocol *dispatchProtocol = nullptr;
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>()) {
        dispatchProtocol = encapsulationProtocolReq->getProtocol(0);
        encapsulationProtocolReq->eraseProtocol(0);
    }
    else if (nextProtocol != nullptr)
        dispatchProtocol = nextProtocol;
    else
        dispatchProtocol = &Protocol::ethernetMac;
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(dispatchProtocol);
}

} // namespace inet

