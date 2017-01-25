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
#include "inet/linklayer/ethernet/EtherEncap.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/common/EtherTypeTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MACAddressTag_m.h"

namespace inet {

Define_Module(EtherEncap);

simsignal_t EtherEncap::encapPkSignal = registerSignal("encapPk");
simsignal_t EtherEncap::decapPkSignal = registerSignal("decapPk");
simsignal_t EtherEncap::pauseSentSignal = registerSignal("pauseSent");

void EtherEncap::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
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
        processFrameFromMAC(check_and_cast<EtherFrame *>(msg));
    }
    else {
        EV_INFO << "Received " << msg << " from upper layer." << endl;
        // from higher layer
        switch (msg->getKind()) {
            case IEEE802CTRL_DATA:
            case 0:    // default message kind (0) is also accepted
                processPacketFromHigherLayer(PK(msg));
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

void EtherEncap::processPacketFromHigherLayer(cPacket *msg)
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
    EtherFrame *frame = nullptr;

    if (useSNAP) {
        EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());

        snapFrame->setSrc(macAddressReq->getSrcAddress());    // if blank, will be filled in by MAC
        snapFrame->setDest(macAddressReq->getDestAddress());
        snapFrame->setOrgCode(0);
        if (etherTypeTag)
            snapFrame->setLocalcode(etherTypeTag->getEtherType());
        frame = snapFrame;
    }
    else {
        EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());

        eth2Frame->setSrc(macAddressReq->getSrcAddress());    // if blank, will be filled in by MAC
        eth2Frame->setDest(macAddressReq->getDestAddress());
        if (etherTypeTag)
            eth2Frame->setEtherType(etherTypeTag->getEtherType());
        frame = eth2Frame;
    }

    ASSERT(frame->getByteLength() > 0); // length comes from msg file

    frame->encapsulate(msg);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"

    EV_INFO << "Sending " << frame << " to lower layer.\n";
    send(frame, "lowerLayerOut");
}

void EtherEncap::processFrameFromMAC(EtherFrame *frame)
{
    // decapsulate and attach control info
    cPacket *higherlayermsg = frame->decapsulate();

    // add Ieee802Ctrl to packet
    auto macAddressInd = higherlayermsg->ensureTag<MacAddressInd>();
    macAddressInd->setSrcAddress(frame->getSrc());
    macAddressInd->setDestAddress(frame->getDest());
    int etherType = -1;
    if (auto eth2frame = dynamic_cast<EthernetIIFrame *>(frame)) {
        etherType = eth2frame->getEtherType();
    }
    else if (auto snapframe = dynamic_cast<EtherFrameWithSNAP *>(frame)) {
        etherType = snapframe->getLocalcode();
    }
    if (etherType != -1) {
        higherlayermsg->ensureTag<EtherTypeInd>()->setEtherType(etherType);
        higherlayermsg->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ethertype.getProtocol(etherType));
    }

    EV_DETAIL << "Decapsulating frame `" << frame->getName() << "', passing up contained packet `"
              << higherlayermsg->getName() << "' to higher layer\n";

    totalFromMAC++;
    emit(decapPkSignal, higherlayermsg);

    // pass up to higher layers.
    EV_INFO << "Sending " << higherlayermsg << " to upper layer.\n";
    send(higherlayermsg, "upperLayerOut");
    delete frame;
}

void EtherEncap::handleSendPause(cMessage *msg)
{
    Ieee802PauseCommand *etherctrl = dynamic_cast<Ieee802PauseCommand *>(msg->removeControlInfo());
    if (!etherctrl)
        throw cRuntimeError("PAUSE command `%s' from higher layer received without Ieee802PauseCommand controlinfo", msg->getName());
    MACAddress dest = etherctrl->getDestinationAddress();
    int pauseUnits = etherctrl->getPauseUnits();
    delete etherctrl;

    EV_DETAIL << "Creating and sending PAUSE frame, with duration = " << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[40];
    sprintf(framename, "pause-%d-%d", getId(), seqNum++);
    EtherPauseFrame *frame = new EtherPauseFrame(framename);
    frame->setPauseTime(pauseUnits);
    if (dest.isUnspecified())
        dest = MACAddress::MULTICAST_PAUSE_ADDRESS;
    frame->setDest(dest);
    frame->setByteLength(ETHER_PAUSE_COMMAND_PADDED_BYTES);

    EV_INFO << "Sending " << frame << " to lower layer.\n";
    send(frame, "lowerLayerOut");
    delete msg;

    emit(pauseSentSignal, pauseUnits);
    totalPauseSent++;
}

} // namespace inet

