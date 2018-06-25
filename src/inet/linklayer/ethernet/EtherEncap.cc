/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(EtherEncap);

simsignal_t EtherEncap::encapPkSignal = registerSignal("encapPk");
simsignal_t EtherEncap::decapPkSignal = registerSignal("decapPk");
simsignal_t EtherEncap::pauseSentSignal = registerSignal("pauseSent");

void EtherEncap::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        fcsMode = parseFcsMode(par("fcsMode"));
        seqNum = 0;
        WATCH(seqNum);
        totalFromHigherLayer = totalFromMAC = totalPauseSent = 0;
        useSNAP = par("useSNAP");
        WATCH(totalFromHigherLayer);
        WATCH(totalFromMAC);
        WATCH(totalPauseSent);
    }
}

void EtherEncap::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("upperLayerIn")) {
        EV_INFO << "Received " << msg << " from upper layer." << endl;
        // from higher layer
        if (msg->isPacket())
            processPacketFromHigherLayer(check_and_cast<Packet *>(msg));
        else
            processCommandFromHigherLayer(msg);
    }
    else if (msg->arrivedOn("lowerLayerIn")) {
        EV_INFO << "Received " << msg << " from lower layer." << endl;
        processFrameFromMAC(check_and_cast<Packet *>(msg));
    }
    else
        throw cRuntimeError("Unknown message");
}

void EtherEncap::processCommandFromHigherLayer(cMessage *msg)
{
    switch (msg->getKind()) {
        case IEEE802CTRL_SENDPAUSE:
            // higher layer want MAC to send PAUSE frame
            handleSendPause(msg);
            break;

        default:
            throw cRuntimeError("Received message `%s' with unknown message kind %d", msg->getName(), msg->getKind());
    }
}

void EtherEncap::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalFromMAC, totalFromHigherLayer);
    getDisplayString().setTagArg("t", 0, buf);
}

void EtherEncap::processPacketFromHigherLayer(Packet *packet)
{
    if (packet->getDataLength() > MAX_ETHERNET_DATA_BYTES)
        throw cRuntimeError("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", (int)packet->getByteLength(), MAX_ETHERNET_DATA_BYTES);

    totalFromHigherLayer++;
    emit(encapPkSignal, packet);

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV_DETAIL << "Encapsulating higher layer packet `" << packet->getName() << "' for MAC\n";

    int typeOrLength = -1;
    if (!useSNAP) {
        auto protocolTag = packet->findTag<PacketProtocolTag>();
        if (protocolTag) {
            const Protocol *protocol = protocolTag->getProtocol();
            if (protocol) {
                int ethType = ProtocolGroup::ethertype.findProtocolNumber(protocol);
                if (ethType != -1)
                    typeOrLength = ethType;
            }
        }
    }
    if (typeOrLength == -1) {
        Ieee8022Llc::encapsulate(packet);
        typeOrLength = packet->getByteLength();
    }
    auto macAddressReq = packet->getTag<MacAddressReq>();
    const auto& ethHeader = makeShared<EthernetMacHeader>();
    ethHeader->setSrc(macAddressReq->getSrcAddress());    // if blank, will be filled in by MAC
    ethHeader->setDest(macAddressReq->getDestAddress());
    ethHeader->setTypeOrLength(typeOrLength);
    packet->insertAtFront(ethHeader);

    EtherEncap::addPaddingAndFcs(packet, fcsMode);

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
    EV_INFO << "Sending " << packet << " to lower layer.\n";
    send(packet, "lowerLayerOut");
}

void EtherEncap::addPaddingAndFcs(Packet *packet, FcsMode fcsMode, B requiredMinBytes)
{
    B paddingLength = requiredMinBytes - ETHER_FCS_BYTES - B(packet->getByteLength());
    if (paddingLength > B(0)) {
        const auto& ethPadding = makeShared<EthernetPadding>();
        ethPadding->setChunkLength(paddingLength);
        packet->insertAtBack(ethPadding);
    }
    addFcs(packet, fcsMode);
}

void EtherEncap::addFcs(Packet *packet, FcsMode fcsMode)
{
    const auto& ethFcs = makeShared<EthernetFcs>();
    ethFcs->setFcsMode(fcsMode);

    // calculate Fcs if needed
    if (fcsMode == FCS_COMPUTED) {
        auto ethBytes = packet->peekDataAsBytes();
        auto bufferLength = B(ethBytes->getChunkLength()).get();
        auto buffer = new uint8_t[bufferLength];
        // 1. fill in the data
        ethBytes->copyToBuffer(buffer, bufferLength);
        // 2. compute the FCS
        auto computedFcs = ethernetCRC(buffer, bufferLength);
        delete [] buffer;
        ethFcs->setFcs(computedFcs);
    }

    packet->insertAtBack(ethFcs);
}

const Ptr<const EthernetMacHeader> EtherEncap::decapsulateMacHeader(Packet *packet)
{
    auto ethHeader = packet->popAtFront<EthernetMacHeader>();
    packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);

    // add Ieee802Ctrl to packet
    auto macAddressInd = packet->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(ethHeader->getSrc());
    macAddressInd->setDestAddress(ethHeader->getDest());

    // remove Padding if possible
    if (isIeee8023Header(*ethHeader)) {
        b payloadLength = B(ethHeader->getTypeOrLength());
        if (packet->getDataLength() < payloadLength)
            throw cRuntimeError("incorrect payload length in ethernet frame");
        packet->setBackOffset(packet->getFrontOffset() + payloadLength);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022);
    }
    else if (isEth2Header(*ethHeader)) {
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(ethHeader->getTypeOrLength()));
    }
    return ethHeader;
}

const Ptr<const EthernetMacHeader> EtherEncap::decapsulateMacLlcSnap(Packet *packet)
{
    const Protocol *payloadProtocol = nullptr;
    auto ethHeader = decapsulateMacHeader(packet);

    // remove llc header if possible
    if (isIeee8023Header(*ethHeader)) {
        this->Ieee8022Llc::decapsulate(packet);
    }
    else if (isEth2Header(*ethHeader)) {
        payloadProtocol = ProtocolGroup::ethertype.getProtocol(ethHeader->getTypeOrLength());
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    }

    return ethHeader;
}

void EtherEncap::processFrameFromMAC(Packet *packet)
{
    // decapsulate and attach control info
    decapsulateMacLlcSnap(packet);

    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);

    EV_DETAIL << "Decapsulating frame `" << packet->getName() << "', passing up contained packet `"
              << packet->getName() << "' to higher layer\n";

    totalFromMAC++;
    emit(decapPkSignal, packet);

    // pass up to higher layers.
    EV_INFO << "Sending " << packet << " to upper layer.\n";
    send(packet, "upperLayerOut");
}

void EtherEncap::handleSendPause(cMessage *msg)
{
    Ieee802PauseCommand *etherctrl = dynamic_cast<Ieee802PauseCommand *>(msg->getControlInfo());
    if (!etherctrl)
        throw cRuntimeError("PAUSE command `%s' from higher layer received without Ieee802PauseCommand controlinfo", msg->getName());
    MacAddress dest = etherctrl->getDestinationAddress();
    int pauseUnits = etherctrl->getPauseUnits();
    delete msg;

    EV_DETAIL << "Creating and sending PAUSE frame, with duration = " << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[40];
    sprintf(framename, "pause-%d-%d", getId(), seqNum++);
    auto packet = new Packet(framename);
    const auto& frame = makeShared<EthernetPauseFrame>();
    const auto& hdr = makeShared<EthernetMacHeader>();
    frame->setPauseTime(pauseUnits);
    if (dest.isUnspecified())
        dest = MacAddress::MULTICAST_PAUSE_ADDRESS;
    hdr->setDest(dest);
    packet->insertAtFront(frame);
    hdr->setTypeOrLength(ETHERTYPE_FLOW_CONTROL);
    packet->insertAtFront(hdr);
    EtherEncap::addPaddingAndFcs(packet, fcsMode);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);

    EV_INFO << "Sending " << frame << " to lower layer.\n";
    send(packet, "lowerLayerOut");

    emit(pauseSentSignal, pauseUnits);
    totalPauseSent++;
}

} // namespace inet

