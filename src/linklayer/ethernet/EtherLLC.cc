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

#include "EtherLLC.h"

#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "Ieee802Ctrl_m.h"


Define_Module(EtherLLC);

simsignal_t EtherLLC::dsapSignal = SIMSIGNAL_NULL;
simsignal_t EtherLLC::encapPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherLLC::decapPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherLLC::passedUpPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherLLC::droppedPkUnknownDSAPSignal = SIMSIGNAL_NULL;
simsignal_t EtherLLC::pauseSentSignal = SIMSIGNAL_NULL;

void EtherLLC::initialize()
{
    seqNum = 0;
    WATCH(seqNum);

    dsapsRegistered = totalFromHigherLayer = totalFromMAC = totalPassedUp = droppedUnknownDSAP = 0;

    dsapSignal = registerSignal("dsap");
    encapPkSignal = registerSignal("encapPk");
    decapPkSignal = registerSignal("decapPk");
    passedUpPkSignal = registerSignal("passedUpPk");
    droppedPkUnknownDSAPSignal = registerSignal("droppedPkUnknownDSAP");
    pauseSentSignal = registerSignal("pauseSent");

    WATCH(dsapsRegistered);
    WATCH(totalFromHigherLayer);
    WATCH(totalFromMAC);
    WATCH(totalPassedUp);
    WATCH(droppedUnknownDSAP);
}

void EtherLLC::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
        // frame received from lower layer
        EtherFrameWithLLC *etherFrameWithLLC = dynamic_cast<EtherFrameWithLLC *>(msg);
        if (etherFrameWithLLC)
        {
            processFrameFromMAC(etherFrameWithLLC);
        }
        else
        {
            EV << "Drop received " << msg->getClassName() << " msg.\n";
            delete msg;
        }
    }
    else
    {
        switch (msg->getKind())
        {
          case IEEE802CTRL_DATA:
            // data received from higher layer
            processPacketFromHigherLayer(PK(msg));
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
            throw cRuntimeError("Received message `%s' with unknown message kind %d",
                  msg->getName(), msg->getKind());
        }
    }

    if (ev.isGUI())
        updateDisplayString();
}

void EtherLLC::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalPassedUp, totalFromHigherLayer);

    if (droppedUnknownDSAP > 0)
    {
        sprintf(buf+strlen(buf), "\ndropped (wrong DSAP): %ld", droppedUnknownDSAP);
    }

    getDisplayString().setTagArg("t", 0, buf);
}

void EtherLLC::processPacketFromHigherLayer(cPacket *msg)
{
    if (msg->getByteLength() > (MAX_ETHERNET_DATA_BYTES-ETHER_LLC_HEADER_LENGTH))
        error("packet from higher layer (%d bytes) plus LLC header exceeds maximum Ethernet payload length (%d)", (int)(msg->getByteLength()), MAX_ETHERNET_DATA_BYTES);

    totalFromHigherLayer++;
    emit(encapPkSignal, msg);

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->getName() <<"' for MAC\n";
    EV << "Sent from " << simulation.getModule(msg->getSenderModuleId())->getFullPath() << " at " << msg->getSendingTime() << " and was created " << msg->getCreationTime() <<  "\n";

    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->getName());

    EtherFrameWithLLC *frame = new EtherFrameWithLLC(msg->getName());

    frame->setControl(0);
    frame->setSsap(etherctrl->getSsap());
    frame->setDsap(etherctrl->getDsap());
    frame->setDest(etherctrl->getDest()); // src address is filled in by MAC
    frame->setByteLength(ETHER_MAC_FRAME_BYTES+ETHER_LLC_HEADER_LENGTH);
    delete etherctrl;

    frame->encapsulate(msg);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    send(frame, "lowerLayerOut");
}

void EtherLLC::processFrameFromMAC(EtherFrameWithLLC *frame)
{
    // check SAP
    int sap = frame->getDsap();
    int port = findPortForSAP(sap);
    if (port < 0)
    {
        EV << "No higher layer registered for DSAP="<< sap <<", discarding frame `" << frame->getName() <<"'\n";
        droppedUnknownDSAP++;
        emit(droppedPkUnknownDSAPSignal, frame);
        delete frame;
        return;
    }

    // decapsulate it and pass up to higher layer
    cPacket *higherlayermsg = frame->decapsulate();

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(frame->getSsap());
    etherctrl->setDsap(frame->getDsap());
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    higherlayermsg->setControlInfo(etherctrl);

    EV << "Decapsulating frame `" << frame->getName() <<"', "
          "passing up contained packet `" << higherlayermsg->getName() << "' "
          "to higher layer " << port << "\n";

    totalFromMAC++;
    emit(decapPkSignal, higherlayermsg);

    // pass up to higher layer
    send(higherlayermsg, "upperLayerOut", port);
    totalPassedUp++;
    emit(passedUpPkSignal, higherlayermsg);
    delete frame;
}

int EtherLLC::findPortForSAP(int dsap)
{
    DsapToPortMap::iterator it = dsapToPort.find(dsap);
    return (it == dsapToPort.end()) ? -1 : it->second;
}

void EtherLLC::handleRegisterSAP(cMessage *msg)
{
    int port = msg->getArrivalGate()->getIndex();
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->getName());
    int dsap = etherctrl->getDsap();

    EV << "Registering higher layer with DSAP=" << dsap << " on port=" << port << "\n";

    if (dsapToPort.find(dsap) != dsapToPort.end())
        error("DSAP=%d already registered with port=%d", dsap, dsapToPort[dsap]);

    dsapToPort[dsap] = port;
    dsapsRegistered = dsapToPort.size();
    emit(dsapSignal, 1L);
    delete msg;
}

void EtherLLC::handleDeregisterSAP(cMessage *msg)
{
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("packet `%s' from higher layer received without Ieee802Ctrl", msg->getName());
    int dsap = etherctrl->getDsap();

    EV << "Deregistering higher layer with DSAP=" << dsap << "\n";

    if (dsapToPort.find(dsap) == dsapToPort.end())
        error("DSAP=%d not registered with port=%d", dsap, dsapToPort[dsap]);

    // delete from table (don't care if it's not in there)
    dsapToPort.erase(dsapToPort.find(dsap));
    dsapsRegistered = dsapToPort.size();
    emit(dsapSignal, -1L);
    delete msg;
}


void EtherLLC::handleSendPause(cMessage *msg)
{
    Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (!etherctrl)
        error("PAUSE command `%s' from higher layer received without Ieee802Ctrl", msg->getName());

    int pauseUnits = etherctrl->getPauseUnits();
    EV << "Creating and sending PAUSE frame, with duration=" << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[40];
    sprintf(framename, "pause-%d-%d", getId(), seqNum++);
    EtherPauseFrame *frame = new EtherPauseFrame(framename);
    frame->setPauseTime(pauseUnits);
    MACAddress dest = etherctrl->getDest();
    if (dest.isUnspecified())
        dest = MACAddress::MULTICAST_PAUSE_ADDRESS;
    frame->setDest(dest);
    frame->setByteLength(ETHER_PAUSE_COMMAND_PADDED_BYTES);

    send(frame, "lowerLayerOut");
    emit(pauseSentSignal, pauseUnits);

    delete msg;
}
