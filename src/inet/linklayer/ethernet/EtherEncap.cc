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
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
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
        const char *fcsModeString = par("fcsMode");
        if (!strcmp(fcsModeString, "declaredCorrect"))
            fcsMode = FCS_DECLARED_CORRECT;
        else if (!strcmp(fcsModeString, "declaredIncorrect"))
            fcsMode = FCS_DECLARED_INCORRECT;
        else if (!strcmp(fcsModeString, "computed"))
            fcsMode = FCS_COMPUTED;
        else
            throw cRuntimeError("Unknown crc mode: '%s'", fcsModeString);

        seqNum = 0;
        WATCH(seqNum);
        totalFromHigherLayer = totalFromMAC = totalPauseSent = 0;
        useSNAP = par("useSNAP").boolValue();
        WATCH(totalFromHigherLayer);
        WATCH(totalFromMAC);
        WATCH(totalPauseSent);
    }
}

void EtherEncap::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn")) {
        EV_INFO << "Received " << msg << " from lower layer." << endl;
        processFrameFromMAC(check_and_cast<Packet *>(msg));
    }
    else {
        EV_INFO << "Received " << msg << " from upper layer." << endl;
        // from higher layer
        switch (msg->getKind()) {
            case IEEE802CTRL_DATA:
            case 0:    // default message kind (0) is also accepted
                processPacketFromHigherLayer(check_and_cast<Packet *>(msg));
                break;

            case IEEE802CTRL_SENDPAUSE:
                // higher layer want MAC to send PAUSE frame
                handleSendPause(msg);
                break;

            default:
                throw cRuntimeError("Received message `%s' with unknown message kind %d", msg->getName(), msg->getKind());
        }
    }
}

void EtherEncap::refreshDisplay() const
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalFromMAC, totalFromHigherLayer);
    getDisplayString().setTagArg("t", 0, buf);
}

void EtherEncap::processPacketFromHigherLayer(Packet *msg)
{
    if (msg->getByteLength() > MAX_ETHERNET_DATA_BYTES)
        throw cRuntimeError("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", (int)msg->getByteLength(), MAX_ETHERNET_DATA_BYTES);

    totalFromHigherLayer++;
    emit(encapPkSignal, msg);

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV_DETAIL << "Encapsulating higher layer packet `" << msg->getName() << "' for MAC\n";

    auto macAddressReq = msg->getMandatoryTag<MacAddressReq>();
    auto etherTypeTag = msg->getTag<EtherTypeReq>();
    int typeOrLength = 0;
    int etherType = (etherTypeTag) ? etherTypeTag->getEtherType() : 0;

    if (useSNAP) {
        const auto& snapHeader = makeShared<Ieee8022LlcSnapHeader>();

        snapHeader->setOui(0);
        snapHeader->setProtocolId(etherType);
        msg->insertHeader(snapHeader);
        typeOrLength = msg->getByteLength();
    }
    else {
        typeOrLength = etherType;
    }
    const auto& ethHeader = makeShared<EthernetMacHeader>();
    ethHeader->setSrc(macAddressReq->getSrcAddress());    // if blank, will be filled in by MAC
    ethHeader->setDest(macAddressReq->getDestAddress());
    ethHeader->setTypeOrLength(typeOrLength);
    msg->insertHeader(ethHeader);

    EtherEncap::addPaddingAndFcs(msg, fcsMode);

    EV_INFO << "Sending " << msg << " to lower layer.\n";
    send(msg, "lowerLayerOut");
}

void EtherEncap::addPaddingAndFcs(Packet *packet, EthernetFcsMode fcsMode, int64_t requiredMinBytes)
{
    int64_t paddingLength = requiredMinBytes - ETHER_FCS_BYTES - packet->getByteLength();
    if (paddingLength > 0) {
        const auto& ethPadding = makeShared<EthernetPadding>();
        ethPadding->setChunkLength(B(paddingLength));
        ethPadding->markImmutable();
        packet->pushTrailer(ethPadding);
    }
    const auto& ethFcs = makeShared<EthernetFcs>();
    ethFcs->setFcsMode(fcsMode);
    //FIXME calculate Fcs if needed
    ethFcs->markImmutable();
    packet->pushTrailer(ethFcs);
}

const Ptr<const EthernetMacHeader> EtherEncap::decapsulateMacHeader(Packet *packet)
{
    auto ethHeader = packet->popHeader<EthernetMacHeader>();
    packet->popTrailer<EthernetFcs>(B(ETHER_FCS_BYTES));
    // remove Padding if possible
    if (isIeee802_3Header(*ethHeader)) {
        b payloadLength = B(ethHeader->getTypeOrLength());
        if (packet->getDataLength() < payloadLength)
            throw cRuntimeError("incorrect payload length in ethernet frame");
        packet->setTrailerPopOffset(packet->getHeaderPopOffset() + payloadLength);
    }
    return ethHeader;
}

const Ptr<const EthernetMacHeader> EtherEncap::decapsulate(Packet *packet, int& outEtherType)
{
    outEtherType = -1;
    auto ethHeader = decapsulateMacHeader(packet);

    // remove llc header if possible
    if (isIeee802_3Header(*ethHeader)) {
        const auto& llcHeader = packet->popHeader<Ieee8022LlcHeader>();
        if (llcHeader->getSsap() == 0xAA && llcHeader->getDsap() == 0xAA && llcHeader->getControl() == 0x03) {
            const auto& snapHeader = dynamicPointerCast<const Ieee8022LlcSnapHeader>(llcHeader);
            if (snapHeader == nullptr)
                throw cRuntimeError("llc/snap error: llc header suggested snap header, but snap header is missing");
            if (snapHeader->getOui() == 0)
                outEtherType = snapHeader->getProtocolId();
        }
        else if (llcHeader->getSsap() == 0x42 && llcHeader->getDsap() == 0x42 && llcHeader->getControl() == 0x03) {
            outEtherType = 0;
        }
    }
    else if (isEth2Header(*ethHeader))
        outEtherType = ethHeader->getTypeOrLength();

    return ethHeader;
}

void EtherEncap::processFrameFromMAC(Packet *packet)
{
    int etherType = -1;
    // decapsulate and attach control info
    const auto& ethHeader = decapsulate(packet, etherType);
    // add Ieee802Ctrl to packet
    auto macAddressInd = packet->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(ethHeader->getSrc());
    macAddressInd->setDestAddress(ethHeader->getDest());
    if (etherType != -1) {
        packet->ensureTag<EtherTypeInd>()->setEtherType(etherType);
        packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
        packet->ensureTag<PacketProtocolTag>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
    }

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
    MACAddress dest = etherctrl->getDestinationAddress();
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
        dest = MACAddress::MULTICAST_PAUSE_ADDRESS;
    hdr->setDest(dest);
    packet->insertHeader(frame);
    hdr->setTypeOrLength(ETHERTYPE_FLOW_CONTROL);
    packet->insertHeader(hdr);
    EtherEncap::addPaddingAndFcs(packet, fcsMode);

   EV_INFO << "Sending " << frame << " to lower layer.\n";
    send(packet, "lowerLayerOut");

    emit(pauseSentSignal, pauseUnits);
    totalPauseSent++;
}

} // namespace inet

