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

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {

Define_Module(Ieee8022Llc);

void Ieee8022Llc::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // TODO: parameterization for llc or snap?
    }
}

void Ieee8022Llc::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        encapsulate(packet);
        send(packet, "lowerLayerOut");
    }
    else if (message->arrivedOn("lowerLayerIn")) {
        auto packet = check_and_cast<Packet *>(message);
        decapsulate(packet);
        send(packet, "upperLayerOut");
    }
    else
        throw cRuntimeError("Unknown message");
}

void Ieee8022Llc::encapsulate(Packet *frame)
{
    if (true) {
        auto ethTypeTag = frame->getTag<EtherTypeReq>();
        const auto& snapHeader = std::make_shared<Ieee8022SnapHeader>();
        snapHeader->setOui(0);
        snapHeader->setProtocolId(ethTypeTag ? ethTypeTag->getEtherType() : -1);
        frame->insertHeader(snapHeader);
    }
    else {
        const auto& llcHeader = std::make_shared<Ieee8022LlcHeader>();
        // TODO: ssap, dsap
        frame->insertHeader(llcHeader);
    }
}

void Ieee8022Llc::decapsulate(Packet *frame)
{
    const auto& llcHeader = frame->popHeader<Ieee8022LlcHeader>();
    if (auto snapHeader = std::dynamic_pointer_cast<Ieee8022SnapHeader>(llcHeader)) {
        int etherType = snapHeader->getProtocolId();
        if (etherType != -1) {
            frame->ensureTag<EtherTypeInd>()->setEtherType(etherType);
            frame->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
            frame->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        }
    }
    else {
        // TODO: ssap, dsap
    }
}

} // namespace inet

