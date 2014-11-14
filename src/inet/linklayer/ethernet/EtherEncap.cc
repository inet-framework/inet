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

#include "inet/linklayer/ethernet/EtherEncap.h"

#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

Define_Module(EtherEncap);

simsignal_t EtherEncap::encapPkSignal = registerSignal("encapPk");
simsignal_t EtherEncap::decapPkSignal = registerSignal("decapPk");
simsignal_t EtherEncap::pauseSentSignal = registerSignal("pauseSent");

void EtherEncap::initialize()
{
    seqNum = 0;
    WATCH(seqNum);

    totalFromHigherLayer = totalFromMAC = totalPauseSent = 0;
    useSNAP = par("useSNAP").boolValue();

    WATCH(totalFromHigherLayer);
    WATCH(totalFromMAC);
    WATCH(totalPauseSent);
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

    if (ev.isGUI())
        updateDisplayString();
}

void EtherEncap::updateDisplayString()
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

    IMACProtocolControlInfo *controlInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(controlInfo);
    EtherFrame *frame = NULL;

    if (useSNAP) {
        EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());

        snapFrame->setSrc(controlInfo->getSourceAddress());    // if blank, will be filled in by MAC
        snapFrame->setDest(controlInfo->getDestinationAddress());
        snapFrame->setOrgCode(0);
        if (etherctrl)
            snapFrame->setLocalcode(etherctrl->getEtherType());
        snapFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH);
        frame = snapFrame;
    }
    else {
        EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());

        eth2Frame->setSrc(controlInfo->getSourceAddress());    // if blank, will be filled in by MAC
        eth2Frame->setDest(controlInfo->getDestinationAddress());
        if (etherctrl)
            eth2Frame->setEtherType(etherctrl->getEtherType());
        eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
        frame = eth2Frame;
    }
    delete controlInfo;

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
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    if (dynamic_cast<EthernetIIFrame *>(frame) != NULL)
        etherctrl->setEtherType(((EthernetIIFrame *)frame)->getEtherType());
    else if (dynamic_cast<EtherFrameWithSNAP *>(frame) != NULL)
        etherctrl->setEtherType(((EtherFrameWithSNAP *)frame)->getLocalcode());
    higherlayermsg->setControlInfo(etherctrl);

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
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        throw cRuntimeError("PAUSE command `%s' from higher layer received without Ieee802Ctrl", msg->getName());
    int pauseUnits = etherctrl->getPauseUnits();
    MACAddress dest = etherctrl->getDest();
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

