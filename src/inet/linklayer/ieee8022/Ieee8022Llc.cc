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
#include "inet/linklayer/common/Ieee802SapTag_m.h"
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
    auto protocolTag = frame->getTag<PacketProtocolTag>();
    const Protocol *protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    int ethType = -1;
    int snapType = -1;
    if (protocol) {
        ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
        if (ethType == -1) {
            snapType = ProtocolGroup::snapOui.findProtocolNumber(protocol);
        }
    }
    if (ethType != -1 || snapType != -1) {
        const auto& snapHeader = makeShared<Ieee8022LlcSnapHeader>();
        if (ethType != -1) {
            snapHeader->setOui(0);
            snapHeader->setProtocolId(ethType);
        }
        else {
            snapHeader->setOui(snapType);
            snapHeader->setProtocolId(-1);      //FIXME get value from a tag (e.g. protocolTag->getSubId() ???)
        }
        frame->insertHeader(snapHeader);
    }
    else {
        const auto& llcHeader = makeShared<Ieee8022LlcHeader>();
        int llctype = ProtocolGroup::ieee8022protocol.findProtocolNumber(protocol);
        if (llctype != -1) {
            llcHeader->setSsap((llctype >> 16) & 0xFF);
            llcHeader->setDsap((llctype >> 8) & 0xFF);
            llcHeader->setControl((llctype) & 0xFF);
        }
        else {
            auto sapTag = frame->getMandatoryTag<Ieee802SapReq>();
            llcHeader->setSsap(sapTag->getSsap());
            llcHeader->setDsap(sapTag->getDsap());
            llcHeader->setControl(3);       //TODO get from sapTag
        }
        frame->insertHeader(llcHeader);
    }
    frame->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022);
}

void Ieee8022Llc::decapsulate(Packet *frame)
{
    const Protocol *payloadProtocol = nullptr;
    const auto& llcHeader = frame->popHeader<Ieee8022LlcHeader>();

    auto sapInd = frame->ensureTag<Ieee802SapInd>();
    sapInd->setSsap(llcHeader->getSsap());
    sapInd->setDsap(llcHeader->getDsap());
    //TODO control?

    int32_t ssapDsapCtrl = (llcHeader->getSsap() << 16) | (llcHeader->getDsap() << 8) | (llcHeader->getControl() & 0xFF);
    if (ssapDsapCtrl == 0xAAAA03) {
        // snap header
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("llc/snap error: llc header suggested snap header, but snap header is missing");
        if (snapHeader->getOui() == 0) {
            payloadProtocol = ProtocolGroup::ethertype.getProtocol(snapHeader->getProtocolId());
        }
        else {
            payloadProtocol = ProtocolGroup::snapOui.getProtocol(snapHeader->getOui());
        }

    }
    else {
        payloadProtocol = ProtocolGroup::ieee8022protocol.findProtocol(ssapDsapCtrl);    // do not use getProtocol
    }
    if (payloadProtocol != nullptr) {
        frame->ensureTag<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    }
    frame->ensureTag<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

} // namespace inet

