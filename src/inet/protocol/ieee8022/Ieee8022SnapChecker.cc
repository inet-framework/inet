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
#include "inet/protocol/ieee8022/Ieee8022SnapChecker.h"

namespace inet {

Define_Module(Ieee8022SnapChecker);

void Ieee8022SnapChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ieee8022snap, nullptr, outputGate);
        registerProtocol(Protocol::ieee8022snap, nullptr, inputGate);
    }
}

bool Ieee8022SnapChecker::matchesPacket(const Packet *packet) const
{
    const auto& snapHeader = packet->peekAtFront<Ieee8022SnapHeader>();
    auto protocol = getProtocol(snapHeader);
    return protocol != nullptr;
}

void Ieee8022SnapChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Unknown protocol, dropping packet\n";
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

void Ieee8022SnapChecker::processPacket(Packet *packet)
{
    const auto& snapHeader = packet->popAtFront<Ieee8022SnapHeader>();
    auto protocol = getProtocol(snapHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
}

const Protocol *Ieee8022SnapChecker::getProtocol(const Ptr<const Ieee8022SnapHeader>& snapHeader)
{
    if (snapHeader->getOui() == 0)
        return ProtocolGroup::ethertype.findProtocol(snapHeader->getProtocolId());
    else
        return ProtocolGroup::snapOui.findProtocol(snapHeader->getOui());
}

} // namespace inet

