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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/Simsignals_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace inet {

Define_Module(Ieee8022Llc);

void Ieee8022Llc::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // TODO: parameterization for llc or snap?
    }
    else if (stage == INITSTAGE_LINK_LAYER)
    {
        if (par("registerProtocol").boolValue()) {    //FIXME //KUDGE should redesign place of EtherEncap and LLC modules
            //register service and protocol
            registerService(Protocol::ieee8022, gate("upperLayerIn"), nullptr);
            registerProtocol(Protocol::ieee8022, nullptr, gate("upperLayerOut"));
        }
    }
}

void Ieee8022Llc::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("upperLayerIn")) {
        // from upper layer
        EV_INFO << "Received " << msg << " from upper layer." << endl;
        if (msg->isPacket())
            processPacketFromHigherLayer(check_and_cast<Packet *>(msg));
        else
            processCommandFromHigherLayer(check_and_cast<Request *>(msg));
    }
    else if (msg->arrivedOn("lowerLayerIn")) {
        EV_INFO << "Received " << msg << " from lower layer." << endl;
        processPacketFromMac(check_and_cast<Packet *>(msg));
    }
    else
        throw cRuntimeError("Unknown gate");
}

void Ieee8022Llc::processPacketFromHigherLayer(Packet *packet)
{
    encapsulate(packet);
    send(packet, "lowerLayerOut");
}

void Ieee8022Llc::processCommandFromHigherLayer(Request *request)
{
    auto ctrl = request->getControlInfo();
    if (ctrl == nullptr)
        throw cRuntimeError("Request '%s' arrived without controlinfo", request->getName());
    else
        throw cRuntimeError("Unknown command: '%s' with %s", request->getName(), ctrl->getClassName());
}

void Ieee8022Llc::processPacketFromMac(Packet *packet)
{
    decapsulate(packet);
    if (packet->getTag<PacketProtocolTag>()->getProtocol() != nullptr || packet->findTag<Ieee802SapInd>() != nullptr)
        send(packet, "upperLayerOut");
    else {
        EV_WARN << "Unknown protocol, dropping packet\n";
        PacketDropDetails details;
        details.setReason(NO_PROTOCOL_FOUND);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Ieee8022Llc::encapsulate(Packet *frame)
{
    auto protocolTag = frame->findTag<PacketProtocolTag>();
    const Protocol *protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    int ethType = -1;
    int snapOui = -1;
    if (protocol) {
        ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
        if (ethType == -1)
            snapOui = ProtocolGroup::snapOui.findProtocolNumber(protocol);
    }
    if (ethType != -1 || snapOui != -1) {
        const auto& snapHeader = makeShared<Ieee8022LlcSnapHeader>();
        if (ethType != -1) {
            snapHeader->setOui(0);
            snapHeader->setProtocolId(ethType);
        }
        else {
            snapHeader->setOui(snapOui);
            snapHeader->setProtocolId(-1);      //FIXME get value from a tag (e.g. protocolTag->getSubId() ???)
        }
        frame->insertAtFront(snapHeader);
    }
    else {
        const auto& llcHeader = makeShared<Ieee8022LlcHeader>();
        int sapData = ProtocolGroup::ieee8022protocol.findProtocolNumber(protocol);
        if (sapData != -1) {
            llcHeader->setSsap((sapData >> 8) & 0xFF);
            llcHeader->setDsap(sapData & 0xFF);
            llcHeader->setControl(3);
        }
        else {
            auto sapReq = frame->getTag<Ieee802SapReq>();
            llcHeader->setSsap(sapReq->getSsap());
            llcHeader->setDsap(sapReq->getDsap());
            llcHeader->setControl(3);       //TODO get from sapTag
        }
        frame->insertAtFront(llcHeader);
    }
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022);
}

void Ieee8022Llc::decapsulate(Packet *frame)
{
    const auto& llcHeader = frame->popAtFront<Ieee8022LlcHeader>();

    auto sapInd = frame->addTagIfAbsent<Ieee802SapInd>();
    sapInd->setSsap(llcHeader->getSsap());
    sapInd->setDsap(llcHeader->getDsap());
    //TODO control?

    if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
        const auto& snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(llcHeader);
        if (snapHeader == nullptr)
            throw cRuntimeError("LLC header indicates SNAP header, but SNAP header is missing");
    }
    auto payloadProtocol = getProtocol(llcHeader);
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

const Protocol *Ieee8022Llc::getProtocol(const Ptr<const Ieee8022LlcHeader>& llcHeader)
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
        payloadProtocol = ProtocolGroup::ieee8022protocol.findProtocol(sapData);    // do not use getProtocol
    }
    return payloadProtocol;
}

} // namespace inet

