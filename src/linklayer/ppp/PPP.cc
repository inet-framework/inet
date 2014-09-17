//
// Copyright (C) 2004 Andras Varga
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

#include <stdio.h>
#include <string.h>

#include "inet/linklayer/ppp/PPP.h"

#include "inet/common/INETUtils.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(PPP);

simsignal_t PPP::txStateSignal = registerSignal("txState");
simsignal_t PPP::rxPkOkSignal = registerSignal("rxPkOk");
simsignal_t PPP::dropPkIfaceDownSignal = registerSignal("dropPkIfaceDown");
simsignal_t PPP::dropPkBitErrorSignal = registerSignal("dropPkBitError");
simsignal_t PPP::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t PPP::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t PPP::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t PPP::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

PPP::PPP()
{
    endTransmissionEvent = NULL;
}

PPP::~PPP()
{
    cancelAndDelete(endTransmissionEvent);
}

void PPP::initialize(int stage)
{
    MACBase::initialize(stage);

    // all initialization is done in the first stage
    if (stage == INITSTAGE_LOCAL) {
        txQueue.setName("txQueue");
        endTransmissionEvent = new cMessage("pppEndTxEvent");

        txQueueLimit = par("txQueueLimit");

        numSent = numRcvdOK = numBitErr = numDroppedIfaceDown = 0;
        WATCH(numSent);
        WATCH(numRcvdOK);
        WATCH(numBitErr);
        WATCH(numDroppedIfaceDown);

        subscribe(POST_MODEL_CHANGE, this);

        emit(txStateSignal, 0L);

        // find queueModule
        queueModule = NULL;

        if (par("queueModule").stringValue()[0]) {
            cModule *mod = getParentModule()->getSubmodule(par("queueModule").stringValue());
            if (mod->isSimple())
                queueModule = check_and_cast<IPassiveQueue *>(mod);
            else {
                cGate *queueOut = mod->gate("out")->getPathStartGate();
                queueModule = check_and_cast<IPassiveQueue *>(queueOut->getOwnerModule());
            }
        }

        // remember the output gate now, to speed up send()
        physOutGate = gate("phys$o");

        // we're connected if other end of connection path is an input gate
        bool connected = physOutGate->getPathEndGate()->getType() == cGate::INPUT;
        // if we're connected, get the gate with transmission rate
        datarateChannel = connected ? physOutGate->getTransmissionChannel() : NULL;
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // register our interface entry in IInterfaceTable
        registerInterface();

        // prepare to fire notifications
        notifDetails.setInterfaceEntry(interfaceEntry);

        // request first frame to send
        if (queueModule && 0 == queueModule->getNumPendingRequests()) {
            EV_DETAIL << "Requesting first frame from queue module\n";
            queueModule->requestPacket();
        }
    }
    // update display string when addresses have been autoconfigured etc.
    else if (stage == INITSTAGE_LAST) {
        // display string stuff
        if (ev.isGUI()) {
            if (datarateChannel) {    // not NULL if connected
                oldConnColor = datarateChannel->getDisplayString().getTagArg("ls", 0);
            }
            else {
                // we are not connected: gray out our icon
                getDisplayString().setTagArg("i", 1, "#707070");
                getDisplayString().setTagArg("i", 2, "100");
            }
        }
        updateDisplayString();
    }
}

InterfaceEntry *PPP::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    e->setName(utils::stripnonalnum(getParentModule()->getFullName()).c_str());

    // data rate
    bool connected = datarateChannel != NULL;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;
    e->setDatarate(datarate);
    e->setCarrier(connected);

    // generate a link-layer address to be used as interface token for IPv6
    InterfaceToken token(0, simulation.getUniqueNumber(), 64);
    e->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setMulticast(true);
    e->setPointToPoint(true);

    return e;
}

void PPP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    MACBase::receiveSignal(source, signalID, obj);

    if (signalID != POST_MODEL_CHANGE)
        return;

    if (dynamic_cast<cPostPathCreateNotification *>(obj)) {
        cPostPathCreateNotification *gcobj = (cPostPathCreateNotification *)obj;
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(true);
    }
    else if (dynamic_cast<cPostPathCutNotification *>(obj)) {
        cPostPathCutNotification *gcobj = (cPostPathCutNotification *)obj;
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(false);
    }
    else if (datarateChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) {
        cPostParameterChangeNotification *gcobj = (cPostParameterChangeNotification *)obj;
        if (datarateChannel == gcobj->par->getOwner() && !strcmp("datarate", gcobj->par->getName()))
            refreshOutGateConnection(true);
    }
}

void PPP::refreshOutGateConnection(bool connected)
{
    Enter_Method_Silent();

    // we're connected if other end of connection path is an input gate
    if (connected)
        ASSERT(physOutGate->getPathEndGate()->getType() == cGate::INPUT);

    if (!connected) {
        if (endTransmissionEvent->isScheduled()) {
            cancelEvent(endTransmissionEvent);

            if (datarateChannel)
                datarateChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
        }

        if (queueModule) {
            // Clear external queue: send a request, and received packet will be deleted in handleMessage()
            if (0 == queueModule->getNumPendingRequests())
                queueModule->requestPacket();
        }
        else {
            //Clear inner queue
            while (!txQueue.empty()) {
                cMessage *msg = check_and_cast<cMessage *>(txQueue.pop());
                EV_ERROR << "Interface is not connected, dropping packet " << msg << endl;
                numDroppedIfaceDown++;
                emit(dropPkIfaceDownSignal, msg);
                delete msg;
            }
        }
    }

    cChannel *oldChannel = datarateChannel;
    // if we're connected, get the gate with transmission rate
    datarateChannel = connected ? physOutGate->getTransmissionChannel() : NULL;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;

    if (datarateChannel && !oldChannel)
        datarateChannel->subscribe(POST_MODEL_CHANGE, this);

    if (ev.isGUI()) {
        if (connected) {
            if (!oldChannel)
                oldConnColor = datarateChannel->getDisplayString().getTagArg("ls", 0);
        }
        else {
            // we are not connected: gray out our icon
            getDisplayString().setTagArg("i", 1, "#707070");
            getDisplayString().setTagArg("i", 2, "100");
        }
    }

    // update interface state if it is in use
    if (interfaceEntry) {
        interfaceEntry->setCarrier(connected);
        interfaceEntry->setDatarate(datarate);
    }

    if (queueModule && 0 == queueModule->getNumPendingRequests())
        queueModule->requestPacket();

    updateDisplayString();
}

void PPP::startTransmitting(cPacket *msg)
{
    // if there's any control info, remove it; then encapsulate the packet
    delete msg->removeControlInfo();
    PPPFrame *pppFrame = encapsulate(msg);

    if (ev.isGUI())
        displayBusy();

    // fire notification
    notifDetails.setPacket(pppFrame);
    emit(NF_PP_TX_BEGIN, &notifDetails);

    // send
    EV_INFO << "Transmission of " << pppFrame << " started.\n";
    emit(txStateSignal, 1L);
    emit(packetSentToLowerSignal, pppFrame);
    send(pppFrame, physOutGate);

    ASSERT(datarateChannel == physOutGate->getTransmissionChannel());    //FIXME reread datarateChannel when changed

    // schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = datarateChannel->getTransmissionFinishTime();
    scheduleAt(endTransmissionTime, endTransmissionEvent);
    numSent++;
}

void PPP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    if (msg == endTransmissionEvent) {
        // Transmission finished, we can start next one.
        EV_INFO << "Transmission successfully completed.\n";
        emit(txStateSignal, 0L);

        if (ev.isGUI())
            displayIdle();

        // fire notification
        notifDetails.setPacket(NULL);
        emit(NF_PP_TX_END, &notifDetails);

        if (!txQueue.empty()) {
            cPacket *pk = (cPacket *)txQueue.pop();
            startTransmitting(pk);
        }
        else if (queueModule && 0 == queueModule->getNumPendingRequests()) {
            queueModule->requestPacket();
        }
    }
    else if (msg->arrivedOn("phys$i")) {
        EV_INFO << "Received " << msg << " from network.\n";
        //TODO: if incoming gate is not connected now, then the link has benn deleted
        // during packet transmission --> discard incomplete packet.

        // fire notification
        notifDetails.setPacket(PK(msg));
        emit(NF_PP_RX_END, &notifDetails);

        emit(packetReceivedFromLowerSignal, msg);

        // check for bit errors
        if (PK(msg)->hasBitError()) {
            EV_WARN << "Bit error in " << msg << endl;
            emit(dropPkBitErrorSignal, msg);
            numBitErr++;
            delete msg;
        }
        else {
            // pass up payload
            PPPFrame *pppFrame = check_and_cast<PPPFrame *>(msg);
            emit(rxPkOkSignal, pppFrame);
            cPacket *payload = decapsulate(pppFrame);
            numRcvdOK++;
            emit(packetSentToUpperSignal, payload);
            EV_INFO << "Sending " << payload << " to upper layer.\n";
            send(payload, "netwOut");
        }
    }
    else {    // arrived on gate "netwIn"
        EV_INFO << "Received " << msg << " from upper layer.\n";
        if (datarateChannel == NULL) {
            EV_WARN << "Interface is not connected, dropping packet " << msg << endl;
            numDroppedIfaceDown++;
            emit(dropPkIfaceDownSignal, msg);
            delete msg;

            if (queueModule && 0 == queueModule->getNumPendingRequests())
                queueModule->requestPacket();
        }
        else {
            emit(packetReceivedFromUpperSignal, msg);

            if (endTransmissionEvent->isScheduled()) {
                // We are currently busy, so just queue up the packet.
                EV_DETAIL << "Received " << msg << " for transmission but transmitter busy, queueing.\n";

                if (ev.isGUI() && txQueue.length() >= 3)
                    getDisplayString().setTagArg("i", 1, "red");

                if (txQueueLimit && txQueue.length() > txQueueLimit)
                    throw cRuntimeError("txQueue length exceeds %d -- this is probably due to "
                                        "a bogus app model generating excessive traffic "
                                        "(or if this is normal, increase txQueueLimit!)",
                            txQueueLimit);

                txQueue.insert(msg);
            }
            else {
                // We are idle, so we can start transmitting right away.
                startTransmitting(PK(msg));
            }
        }
    }

    if (ev.isGUI())
        updateDisplayString();
}

void PPP::displayBusy()
{
    getDisplayString().setTagArg("i", 1, txQueue.length() >= 3 ? "red" : "yellow");
    datarateChannel->getDisplayString().setTagArg("ls", 0, "yellow");
    datarateChannel->getDisplayString().setTagArg("ls", 1, "3");
}

void PPP::displayIdle()
{
    getDisplayString().setTagArg("i", 1, "");

    if (datarateChannel) {
        datarateChannel->getDisplayString().setTagArg("ls", 0, oldConnColor.c_str());
        datarateChannel->getDisplayString().setTagArg("ls", 1, "1");
    }
}

void PPP::updateDisplayString()
{
    if (ev.isDisabled()) {
        // speed up things
        getDisplayString().setTagArg("t", 0, "");
    }
    else if (datarateChannel != NULL) {
        char datarateText[40];

        double datarate = datarateChannel->getNominalDatarate();
        if (datarate >= 1e9)
            sprintf(datarateText, "%gGbps", datarate / 1e9);
        else if (datarate >= 1e6)
            sprintf(datarateText, "%gMbps", datarate / 1e6);
        else if (datarate >= 1e3)
            sprintf(datarateText, "%gkbps", datarate / 1e3);
        else
            sprintf(datarateText, "%gbps", datarate);

/* TBD find solution for displaying IPv4 address without dependence on IPv4 or IPv6
        IPv4Address addr = interfaceEntry->ipv4Data()->getIPAddress();
        sprintf(buf, "%s / %s\nrcv:%ld snt:%ld", addr.isUnspecified()?"-":addr.str().c_str(), datarateText, numRcvdOK, numSent);
 */

        char buf[80];
        sprintf(buf, "%s\nrcv:%ld snt:%ld", datarateText, numRcvdOK, numSent);

        if (numBitErr > 0)
            sprintf(buf + strlen(buf), "\nerr:%ld", numBitErr);

        getDisplayString().setTagArg("t", 0, buf);
    }
    else {
        char buf[80];
        sprintf(buf, "not connected\ndropped:%ld", numDroppedIfaceDown);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

PPPFrame *PPP::encapsulate(cPacket *msg)
{
    PPPFrame *pppFrame = new PPPFrame(msg->getName());
    pppFrame->setByteLength(PPP_OVERHEAD_BYTES);
    pppFrame->encapsulate(msg);
    return pppFrame;
}

cPacket *PPP::decapsulate(PPPFrame *pppFrame)
{
    cPacket *msg = pppFrame->decapsulate();
    delete pppFrame;
    return msg;
}

void PPP::flushQueue()
{
    // code would look slightly nicer with a pop() function that returns NULL if empty
    if (queueModule) {
        while (!queueModule->isEmpty()) {
            cMessage *msg = queueModule->pop();
            emit(dropPkIfaceDownSignal, msg);    //FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete msg;
        }
        queueModule->clear();    // clear request count
        queueModule->requestPacket();
    }
    else {
        while (!txQueue.empty()) {
            cMessage *msg = (cMessage *)txQueue.pop();
            emit(dropPkIfaceDownSignal, msg);    //FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete msg;
        }
    }
}

void PPP::clearQueue()
{
    // code would look slightly nicer with a pop() function that returns NULL if empty
    if (queueModule) {
        queueModule->clear();    // clear request count
        queueModule->requestPacket();
    }
    else {
        txQueue.clear();
    }
}

} // namespace inet

