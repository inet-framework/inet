//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/protocol/ieee8022/Ieee8022LlcChecker.h"

namespace inet {

Define_Module(Ieee8022LlcChecker);

void Ieee8022LlcChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ieee8022, nullptr, inputGate);
        registerProtocol(Protocol::ieee8022, nullptr, outputGate);
    }
}

bool Ieee8022LlcChecker::matchesPacket(const Packet *packet) const
{
    const auto& llcHeader = packet->peekAtFront<Ieee8022LlcHeader>();
    auto protocol = getProtocol(llcHeader);
    return protocol != nullptr;
}

void Ieee8022LlcChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Unknown protocol, dropping packet\n";
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

void Ieee8022LlcChecker::processPacket(Packet *packet)
{
    const auto& llcHeader = packet->popAtFront<Ieee8022LlcHeader>();

    auto sapInd = packet->addTagIfAbsent<Ieee802SapInd>();
    sapInd->setSsap(llcHeader->getSsap());
    sapInd->setDsap(llcHeader->getDsap());
    //TODO control?

    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
    }
    auto payloadProtocol = getProtocol(llcHeader);
    if (payloadProtocol) {
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    }
    else {
        delete packet->removeTagIfPresent<DispatchProtocolReq>();
        delete packet->removeTagIfPresent<PacketProtocolTag>();
    }
}

const Protocol *Ieee8022LlcChecker::getProtocol(const Ptr<const Ieee8022LlcHeader>& llcHeader)
{
    const Protocol *payloadProtocol = nullptr;
    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
        if (snapHeader->getOui() == 0)
            payloadProtocol = ProtocolGroup::ethertype.findProtocol(snapHeader->getProtocolId());
        else
            payloadProtocol = ProtocolGroup::snapOui.findProtocol(snapHeader->getOui());
    }
    else {
        int32_t sapData = ((llcHeader->getSsap() & 0xFF) << 8) | (llcHeader->getDsap() & 0xFF);
        payloadProtocol = ProtocolGroup::ieee8022protocol.findProtocol(sapData);
    }
    return payloadProtocol;
}

} // namespace inet

