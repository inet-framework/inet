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
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/protocol/ieee8022/Ieee8022SnapInserter.h"

namespace inet {

Define_Module(Ieee8022SnapInserter);

void Ieee8022SnapInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(Protocol::ieee8022snap, inputGate, nullptr);
        registerProtocol(Protocol::ieee8022snap, outputGate, outputGate);
    }
}

void Ieee8022SnapInserter::processPacket(Packet *packet)
{
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    int ethType = -1;
    int snapOui = -1;
    if (protocol) {
        ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
        if (ethType == -1)
            snapOui = ProtocolGroup::snapOui.findProtocolNumber(protocol);
    }
    const auto& snapHeader = makeShared<Ieee8022SnapHeader>();
    if (ethType != -1) {
        snapHeader->setOui(0);
        snapHeader->setProtocolId(ethType);
    }
    else {
        snapHeader->setOui(snapOui);
        snapHeader->setProtocolId(-1);      //FIXME get value from a tag (e.g. protocolTag->getSubId() ???)
    }
    packet->insertAtFront(snapHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022snap);
}

const Protocol *Ieee8022SnapInserter::getProtocol(const Ptr<const Ieee8022SnapHeader>& snapHeader)
{
    if (snapHeader->getOui() == 0)
        return ProtocolGroup::ethertype.findProtocol(snapHeader->getProtocolId());
    else
        return ProtocolGroup::snapOui.findProtocol(snapHeader->getOui());
}

} // namespace inet

