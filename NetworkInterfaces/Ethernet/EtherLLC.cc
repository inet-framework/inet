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
#include <map>
#include <omnetpp.h>
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "Ieee802Ctrl_m.h"
#include "utils.h"


/**
 * Implements the LLC sub-layer of the Datalink Layer in Ethernet networks
 */
class INET_API EtherLLC : public cSimpleModule
{
  protected:
    int seqNum;
    std::map<int,int> dsapToPort;  // DSAP registration table

    // statistics
    long dsapsRegistered;       // number DSAPs (higher layers) registered
    long totalFromHigherLayer;  // total number of packets received from higher layer
    long totalFromMAC;          // total number of frames received from MAC
    long totalPassedUp;         // total number of packets passed up to higher layer
    long droppedUnknownDSAP;    // frames dropped because no such DSAP was registered here

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void processPacketFromHigherLayer(cMessage *msg);
    virtual void processFrameFromMAC(EtherFrameWithLLC *msg);
    virtual void handleRegisterSAP(cMessage *msg);
    virtual void handleDeregisterSAP(cMessage *msg);
    virtual void handleSendPause(cMessage *msg);
    virtual int findPortForSAP(int sap);

    // utility function
    void updateDisplayString();
};

Define_Module(EtherLLC);

void EtherLLC::initialize()
{
    seqNum = 0;
    WATCH(seqNum);

    dsapsRegistered = totalFromHigherLayer = totalFromMAC = totalPassedUp = droppedUnknownDSAP = 0;
    WATCH(dsapsRegistered);
    WATCH(totalFromHigherLayer);
    WATCH(totalFromMAC);
    WATCH(totalPassedUp);
    WATCH(droppedUnknownDSAP);
}

void EtherLLC::handleMessage(cMessage *msg)
{
    switch (msg->kind())
    {
      case IEEE802CTRL_DATA:
        // data received from higher layer
        processPacketFromHigherLayer(msg);
        break;

      case ETH_FRAME:
        // frame received from lower layer
        processFrameFromMAC(check_and_cast<EtherFrameWithLLC *>(msg));
        break;

      case IEEE802CTRL_REGISTER_DSAP:
        // higher layer registers itself
        handleRegisterSAP(msg);
        break;

      case IEEE802CTRL_DEREGISTER_DSAP:
        // higher layer deregisters itself
        handleDeregisterSAP(msg);
        break;

      case IEEE802CTRL_SENDPAUSE:
        // higher layer want MAC to send PAUSE frame
        handleSendPause(msg);
        break;

      default:
        error("received message `%s' with unknown message kind %d",
              msg->name(), msg->kind());
    }

    if (ev.isGUI())
        updateDisplayString();
}

void EtherLLC::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalPassedUp, totalFromHigherLayer);
    if (droppedUnknownDSAP>0)
    {
        sprintf(buf+strlen(buf), "\ndropped (wrong DSAP): %ld", droppedUnknownDSAP);
    }
    displayString().setTagArg("t",0,buf);
}

void EtherLLC::processPacketFromHigherLayer(cMessage *msg)
{
    if (msg->byteLength() > (MAX_ETHERNET_DATA-ETHER_LLC_HEADER_LENGTH))
        error("packet from higher layer (%d bytes) plus LLC header exceed maximum Ethernet payload length (%d)", msg->byteLength(), MAX_ETHERNET_DATA);

    totalFromHigherLayer++;

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->name() <<"' for MAC\n";
    EV << "Sent from " << simulation.module(msg->senderModuleId())->fullPath() << " at " << msg->sendingTime() << " and was created " << msg->creationTime() <<  "\n";

    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->name());

    EtherFrameWithLLC *frame = new EtherFrameWithLLC(msg->name(), ETH_FRAME);

    frame->setControl(0);
    frame->setSsap(etherctrl->getSsap());
    frame->setDsap(etherctrl->getDsap());
    frame->setDest(etherctrl->getDest()); // src address is filled in by MAC
    frame->setByteLength(ETHER_MAC_FRAME_BYTES+ETHER_LLC_HEADER_LENGTH);
    delete etherctrl;

    frame->encapsulate(msg);
    if (frame->byteLength() < MIN_ETHERNET_FRAME)
        frame->setByteLength(MIN_ETHERNET_FRAME);

    send(frame, "lowerLayerOut");
}

void EtherLLC::processFrameFromMAC(EtherFrameWithLLC *frame)
{
    totalFromMAC++;

    // decapsulate it and pass up to higher layers.
    int sap = frame->getDsap();
    int port = findPortForSAP(sap);
    if (port<0)
    {
        EV << "No higher layer registered for DSAP="<< sap <<", discarding frame `" << frame->name() <<"'\n";
        droppedUnknownDSAP++;
        delete frame;
        return;
    }

    cMessage *higherlayermsg = frame->decapsulate();

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(frame->getSsap());
    etherctrl->setDsap(frame->getDsap());
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    higherlayermsg->setControlInfo(etherctrl);

    EV << "Decapsulating frame `" << frame->name() <<"', "
          "passing up contained packet `" << higherlayermsg->name() << "' "
          "to higher layer " << port << "\n";

    send(higherlayermsg, "upperLayerOut", port);
    totalPassedUp++;
    delete frame;
}

int EtherLLC::findPortForSAP(int dsap)
{
    // here we actually do two lookups, but what the hell...
    if (dsapToPort.find(dsap)==dsapToPort.end())
        return -1;
    return dsapToPort[dsap];
}

void EtherLLC::handleRegisterSAP(cMessage *msg)
{
    int port = msg->arrivalGate()->index();
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->name());
    int dsap = etherctrl->getDsap();

    EV << "Registering higher layer with DSAP=" << dsap << " on port=" << port << "\n";

    if (dsapToPort.find(dsap)!=dsapToPort.end())
        error("DSAP=%d already registered with port=%d", dsap, dsapToPort[dsap]);

    dsapToPort[dsap] = port;
    dsapsRegistered = dsapToPort.size();
    delete msg;
}

void EtherLLC::handleDeregisterSAP(cMessage *msg)
{
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->name());
    int dsap = etherctrl->getDsap();

    EV << "Deregistering higher layer with DSAP=" << dsap << "\n";

    // delete from table (don't care if it's not in there)
    dsapToPort.erase(dsapToPort.find(dsap));
    dsapsRegistered = dsapToPort.size();
    delete msg;
}


void EtherLLC::handleSendPause(cMessage *msg)
{
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("PAUSE command `%s' from higher layer received without Ieee802Ctrl", msg->name());

    int pauseUnits = etherctrl->getPauseUnits();
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
}

void EtherLLC::finish()
{
    if (par("writeScalars").boolValue())
    {
        recordScalar("dsaps registered", dsapsRegistered);
        recordScalar("packets from higher layer", totalFromHigherLayer);
        recordScalar("frames from MAC", totalFromMAC);
        recordScalar("packets passed up", totalPassedUp);
        recordScalar("packets dropped - unknown DSAP", droppedUnknownDSAP);
    }
}

