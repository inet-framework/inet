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

#include "PPP.h"

#include "opp_utils.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"


Define_Module(PPP);

simsignal_t PPP::txStateSignal = SIMSIGNAL_NULL;
simsignal_t PPP::rxPkOkSignal = SIMSIGNAL_NULL;
simsignal_t PPP::dropPkIfaceDownSignal = SIMSIGNAL_NULL;
simsignal_t PPP::dropPkBitErrorSignal = SIMSIGNAL_NULL;
simsignal_t PPP::packetSentToLowerSignal = SIMSIGNAL_NULL;
simsignal_t PPP::packetReceivedFromLowerSignal = SIMSIGNAL_NULL;
simsignal_t PPP::packetSentToUpperSignal = SIMSIGNAL_NULL;
simsignal_t PPP::packetReceivedFromUpperSignal = SIMSIGNAL_NULL;

PPP::PPP()
{
    endTransmissionEvent = NULL;
    nb = NULL;
}

PPP::~PPP()
{
    // kludgy way to check that nb is not deleted yet
    if (nb && nb == findModuleWhereverInNode("notificationBoard", this))
        nb->unsubscribe(this, NF_SUBSCRIBERLIST_CHANGED);

    cancelAndDelete(endTransmissionEvent);
}

void PPP::initialize(int stage)
{
    // all initialization is done in the first stage
    if (stage == 0)
    {
        txQueue.setName("txQueue");
        endTransmissionEvent = new cMessage("pppEndTxEvent");

        txQueueLimit = par("txQueueLimit");

        interfaceEntry = NULL;

        numSent = numRcvdOK = numBitErr = numDroppedIfaceDown = 0;
        WATCH(numSent);
        WATCH(numRcvdOK);
        WATCH(numBitErr);
        WATCH(numDroppedIfaceDown);

        rxPkOkSignal = registerSignal("rxPkOk");
        dropPkIfaceDownSignal = registerSignal("dropPkIfaceDown");
        txStateSignal = registerSignal("txState");
        dropPkBitErrorSignal = registerSignal("dropPkBitError");
        packetSentToLowerSignal = registerSignal("packetSentToLower");
        packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
        packetSentToUpperSignal = registerSignal("packetSentToUpper");
        packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

        subscribe(POST_MODEL_CHANGE, this);

        emit(txStateSignal, 0L);

        // find queueModule
        queueModule = NULL;

        if (par("queueModule").stringValue()[0])
        {
            cModule *mod = getParentModule()->getSubmodule(par("queueModule").stringValue());
            if (mod->isSimple())
                queueModule = check_and_cast<IPassiveQueue *>(mod);
            else
            {
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
        double datarate = connected ? datarateChannel->getNominalDatarate() : 0;

        // register our interface entry in IInterfaceTable
        interfaceEntry = registerInterface(datarate);
        interfaceEntry->setDown(!connected);

        // prepare to fire notifications
        nb = NotificationBoardAccess().get();
        notifDetails.setInterfaceEntry(interfaceEntry);
        nb->subscribe(this, NF_SUBSCRIBERLIST_CHANGED);
        updateHasSubcribers();

        // display string stuff
        if (ev.isGUI())
        {
            if (connected)
            {
                oldConnColor = datarateChannel->getDisplayString().getTagArg("ls", 0);
            }
            else
            {
                // we are not connected: gray out our icon
                getDisplayString().setTagArg("i", 1, "#707070");
                getDisplayString().setTagArg("i", 2, "100");
            }
        }

        // request first frame to send
        if (queueModule && 0 == queueModule->getNumPendingRequests())
        {
            EV << "Requesting first frame from queue module\n";
            queueModule->requestPacket();
        }
    }

    // update display string when addresses have been autoconfigured etc.
    if (stage == 3)
    {
        updateDisplayString();
    }
}

InterfaceEntry *PPP::registerInterface(double datarate)
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    e->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

    // data rate
    e->setDatarate(datarate);

    // generate a link-layer address to be used as interface token for IPv6
    InterfaceToken token(0, simulation.getUniqueNumber(), 64);
    e->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setMulticast(true);
    e->setPointToPoint(true);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (ift)
        ift->addInterface(e);

    return e;
}

void PPP::receiveSignal(cComponent *src, simsignal_t id, cObject *obj)
{
    if (dynamic_cast<cPostPathCreateNotification *>(obj))
    {
        cPostPathCreateNotification *gcobj = (cPostPathCreateNotification *)obj;
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(true);
    }
    else if (dynamic_cast<cPostPathCutNotification *>(obj))
    {
        cPostPathCutNotification *gcobj = (cPostPathCutNotification *)obj;
        if (physOutGate == gcobj->pathStartGate)
            refreshOutGateConnection(false);
    }
    else if (datarateChannel && dynamic_cast<cPostParameterChangeNotification *>(obj))
    {
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

    if (!connected)
    {
        if (endTransmissionEvent->isScheduled())
        {
            cancelEvent(endTransmissionEvent);

            if (datarateChannel)
                datarateChannel->forceTransmissionFinishTime(SIMTIME_ZERO);
        }

        if (queueModule)
        {
            // Clear external queue: send a request, and received packet will be deleted in handleMessage()
            if (0 == queueModule->getNumPendingRequests())
                queueModule->requestPacket();
        }
        else
        {
            //Clear inner queue
            while (!txQueue.empty())
            {
                cMessage *msg = check_and_cast<cMessage *>(txQueue.pop());
                EV << "Interface is not connected, dropping packet " << msg << endl;
                numDroppedIfaceDown++;
                emit(dropPkIfaceDownSignal, msg);
                delete msg;
            }
        }
    }

    cChannel* oldChannel = datarateChannel;
    // if we're connected, get the gate with transmission rate
    datarateChannel = connected ? physOutGate->getTransmissionChannel() : NULL;
    double datarate = connected ? datarateChannel->getNominalDatarate() : 0;

    if (datarateChannel && !oldChannel)
        datarateChannel->subscribe(POST_MODEL_CHANGE, this);

    if (ev.isGUI())
    {
        if (connected)
        {
            if (!oldChannel)
                oldConnColor = datarateChannel->getDisplayString().getTagArg("ls", 0);
        }
        else
        {
            // we are not connected: gray out our icon
            getDisplayString().setTagArg("i", 1, "#707070");
            getDisplayString().setTagArg("i", 2, "100");
        }
    }

    // set interface state
    interfaceEntry->setDown(!connected);

    // data rate
    interfaceEntry->setDatarate(datarate);

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

    if (hasSubscribers)
    {
        // fire notification
        notifDetails.setPacket(pppFrame);
        nb->fireChangeNotification(NF_PP_TX_BEGIN, &notifDetails);
    }

    // send
    EV << "Starting transmission of " << pppFrame << endl;
    emit(txStateSignal, 1L);
    emit(packetSentToLowerSignal, pppFrame);
    send(pppFrame, physOutGate);

    ASSERT(datarateChannel == physOutGate->getTransmissionChannel()); //FIXME reread datarateChannel when changed

    // schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmissionTime = datarateChannel->getTransmissionFinishTime();
    scheduleAt(endTransmissionTime, endTransmissionEvent);
    numSent++;
}

void PPP::handleMessage(cMessage *msg)
{
    if (msg==endTransmissionEvent)
    {
        // Transmission finished, we can start next one.
        EV << "Transmission finished.\n";
        emit(txStateSignal, 0L);

        if (ev.isGUI())
            displayIdle();

        if (hasSubscribers)
        {
            // fire notification
            notifDetails.setPacket(NULL);
            nb->fireChangeNotification(NF_PP_TX_END, &notifDetails);
        }

        if (!txQueue.empty())
        {
            cPacket *pk = (cPacket *)txQueue.pop();
            startTransmitting(pk);
        }
        else if (queueModule && 0 == queueModule->getNumPendingRequests())
        {
            queueModule->requestPacket();
        }
    }
    else if (msg->arrivedOn("phys$i"))
    {
        //TODO: if incoming gate is not connected now, then the link has benn deleted
        // during packet transmission --> discard incomplete packet.

        if (hasSubscribers)
        {
            // fire notification
            notifDetails.setPacket(PK(msg));
            nb->fireChangeNotification(NF_PP_RX_END, &notifDetails);
        }

        emit(packetReceivedFromLowerSignal, msg);

        // check for bit errors
        if (PK(msg)->hasBitError())
        {
            EV << "Bit error in " << msg << endl;
            emit(dropPkBitErrorSignal, msg);
            numBitErr++;
            delete msg;
        }
        else
        {
            // pass up payload
            PPPFrame *pppFrame = check_and_cast<PPPFrame *>(msg);
            emit(rxPkOkSignal, pppFrame);
            cPacket *payload = decapsulate(pppFrame);
            numRcvdOK++;
            emit(packetSentToUpperSignal, payload);
            send(payload, "netwOut");
        }
    }
    else // arrived on gate "netwIn"
    {
        if (datarateChannel == NULL)
        {
            EV << "Interface is not connected, dropping packet " << msg << endl;
            numDroppedIfaceDown++;
            emit(dropPkIfaceDownSignal, msg);
            delete msg;

            if (queueModule && 0 == queueModule->getNumPendingRequests())
                queueModule->requestPacket();
        }
        else
        {
            emit(packetReceivedFromUpperSignal, msg);

            if (endTransmissionEvent->isScheduled())
            {
                // We are currently busy, so just queue up the packet.
                EV << "Received " << msg << " for transmission but transmitter busy, queueing.\n";

                if (ev.isGUI() && txQueue.length() >= 3)
                    getDisplayString().setTagArg("i", 1, "red");

                if (txQueueLimit && txQueue.length() > txQueueLimit)
                    error("txQueue length exceeds %d -- this is probably due to "
                          "a bogus app model generating excessive traffic "
                          "(or if this is normal, increase txQueueLimit!)",
                          txQueueLimit);

                txQueue.insert(msg);
            }
            else
            {
                // We are idle, so we can start transmitting right away.
                EV << "Received " << msg << " for transmission\n";
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

    if (datarateChannel)
    {
        datarateChannel->getDisplayString().setTagArg("ls", 0, oldConnColor.c_str());
        datarateChannel->getDisplayString().setTagArg("ls", 1, "1");
    }
}

void PPP::updateDisplayString()
{
    if (ev.isDisabled())
    {
        // speed up things
        getDisplayString().setTagArg("t", 0, "");
    }
    else if (datarateChannel != NULL)
    {
        char datarateText[40];

        double datarate = datarateChannel->getNominalDatarate();
        if (datarate >= 1e9) sprintf(datarateText, "%gGbps", datarate / 1e9);
        else if (datarate >= 1e6) sprintf(datarateText, "%gMbps", datarate / 1e6);
        else if (datarate >= 1e3) sprintf(datarateText, "%gkbps", datarate / 1e3);
        else sprintf(datarateText, "%gbps", datarate);

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
    else
    {
        char buf[80];
        sprintf(buf, "not connected\ndropped:%ld", numDroppedIfaceDown);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void PPP::updateHasSubcribers()
{
    hasSubscribers = nb->hasSubscribers(NF_PP_TX_BEGIN) ||
                     nb->hasSubscribers(NF_PP_TX_END) ||
                     nb->hasSubscribers(NF_PP_RX_END);
}

void PPP::receiveChangeNotification(int category, const cObject *)
{
    if (category == NF_SUBSCRIBERLIST_CHANGED)
        updateHasSubcribers();
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

