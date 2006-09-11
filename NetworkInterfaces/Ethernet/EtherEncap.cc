/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <omnetpp.h>
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "Ieee802Ctrl_m.h"
#include "utils.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "EtherMAC.h"


/**
 * Performs Ethernet II encapsulation/decapsulation. More info in the NED file.
 */
class INET_API EtherEncap : public cSimpleModule
{
  protected:
    int seqNum;

    // statistics
    long totalFromHigherLayer;  // total number of packets received from higher layer
    long totalFromMAC;          // total number of frames received from MAC
    long totalPauseSent;        // total number of PAUSE frames sent

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void processPacketFromHigherLayer(cMessage *msg);
    virtual void processFrameFromMAC(EtherFrame *msg);
    virtual void handleSendPause(cMessage *msg);

    virtual void updateDisplayString();
};

Define_Module(EtherEncap);

void EtherEncap::initialize()
{
    seqNum = 0;
    WATCH(seqNum);

    totalFromHigherLayer = totalFromMAC = totalPauseSent = 0;
    WATCH(totalFromHigherLayer);
    WATCH(totalFromMAC);
    WATCH(totalPauseSent);
}

void EtherEncap::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
        processFrameFromMAC(check_and_cast<EtherFrame *>(msg));
    }
    else
    {
        // from higher layer
        switch(msg->kind())
        {
            case IEEE802CTRL_DATA:
            case 0: // default message kind (0) is also accepted
              processPacketFromHigherLayer(msg);
              break;

            case IEEE802CTRL_SENDPAUSE:
              // higher layer want MAC to send PAUSE frame
              handleSendPause(msg);
              break;

            default:
              error("received message `%s' with unknown message kind %d", msg->name(), msg->kind());
        }
    }

    if (ev.isGUI())
        updateDisplayString();
}

void EtherEncap::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalFromMAC, totalFromHigherLayer);
    displayString().setTagArg("t",0,buf);
}

void EtherEncap::processPacketFromHigherLayer(cMessage *msg)
{
    if (msg->byteLength() > MAX_ETHERNET_DATA)
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", msg->byteLength(), MAX_ETHERNET_DATA);

    totalFromHigherLayer++;

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->name() <<"' for MAC\n";

    Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
    EthernetIIFrame *frame = new EthernetIIFrame(msg->name(), ETH_FRAME);

    frame->setSrc(etherctrl->getSrc());  // if blank, will be filled in by MAC
    frame->setDest(etherctrl->getDest());
    frame->setEtherType(etherctrl->getEtherType());
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    delete etherctrl;

    frame->encapsulate(msg);
    if (frame->byteLength() < MIN_ETHERNET_FRAME)
        frame->setByteLength(MIN_ETHERNET_FRAME);  // "padding"

    send(frame, "lowerLayerOut");
}

void EtherEncap::processFrameFromMAC(EtherFrame *frame)
{
    totalFromMAC++;

    // decapsulate and attach control info
    cMessage *higherlayermsg = frame->decapsulate();

    // add Ieee802Ctrl to packet
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    higherlayermsg->setControlInfo(etherctrl);

    EV << "Decapsulating frame `" << frame->name() <<"', passing up contained "
          "packet `" << higherlayermsg->name() << "' to higher layer\n";

    // pass up to higher layers.
    send(higherlayermsg, "upperLayerOut");
    delete frame;
}

void EtherEncap::handleSendPause(cMessage *msg)
{
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl*>(msg->removeControlInfo());
    if (!etherctrl)
        error("PAUSE command `%s' from higher layer received without Ieee802Ctrl", msg->name());
    int pauseUnits = etherctrl->getPauseUnits();
    delete etherctrl;

    EV << "Creating and sending PAUSE frame, with duration=" << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[30];
    sprintf(framename, "pause-%d-%d", id(), seqNum++);
    EtherPauseFrame *frame = new EtherPauseFrame(framename, ETH_PAUSE);
    frame->setPauseTime(pauseUnits);

    frame->setByteLength(ETHER_MAC_FRAME_BYTES+ETHER_PAUSE_COMMAND_BYTES);
    if (frame->byteLength() < MIN_ETHERNET_FRAME)
        frame->setByteLength(MIN_ETHERNET_FRAME);

    send(frame, "lowerLayerOut");
    delete msg;

    totalPauseSent++;
}

void EtherEncap::finish()
{
    if (par("writeScalars").boolValue())
    {
        recordScalar("packets from higher layer", totalFromHigherLayer);
        recordScalar("frames from MAC", totalFromMAC);
    }
}


