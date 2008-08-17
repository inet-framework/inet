/* -*- mode:c++ -*- ********************************************************
 * file:        CSMAMacLayer.cc
 *
 * author:      Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#include "CSMAMacLayer.h"
#include "Ieee802Ctrl_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"


Define_Module(CSMAMacLayer);


CSMAMacLayer::CSMAMacLayer()
{
    timer = NULL;
}

CSMAMacLayer::~CSMAMacLayer()
{
    cancelAndDelete(timer);
}

void CSMAMacLayer::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0)
    {
        queueLength = hasPar("queueLength") ? (int)par("queueLength") : 0;
        EV << "queueLength = " << queueLength << endl;

        //subscribe for the information of the carrier sense
        nb->subscribe(this, NF_RADIOSTATE_CHANGED);

        // initialize the timer
        timer = new cMessage("backoff");

        radioState = RadioState::IDLE; // until 1st receiveChangeNotification()

        // get registered in IInterfaceTable
        registerInterface();
    }
}

void CSMAMacLayer::registerInterface()
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    const char *addrstr = par("address");
    if (!strcmp(addrstr, "auto"))
    {
        // assign automatic address
        myMacAddr = MACAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(myMacAddr.str().c_str());
    }
    else
    {
        myMacAddr.setAddress(addrstr);
    }
    e->setMACAddress(myMacAddr);

    // generate interface identifier for IPv6
    e->setInterfaceToken(myMacAddr.formInterfaceIdentifier());

    // MTU on 802.11 = ?
    e->setMtu(1500);  // FIXME

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().get();
    ift->addInterface(e, this);
}


void CSMAMacLayer::finish()
{
}

void CSMAMacLayer::handleCommand(cMessage *msg)
{
    // no commands supported by CSMAMacLayer
    error("Non-packet message arrived from higher layer: (%s)%s", msg->getClassName(), msg->getName());
}

/**
 * First it has to be checked whether a frame is currently being
 * transmitted or waiting to be transmitted. If so the newly arrived
 * message is stored in a queue. If there is no queue or it is full
 * print a warning.
 *
 * Before transmitting a frame it is tested whether the channel
 * is busy at the moment or not. If the channel is busy, a short
 * random time will be generated and the MacPkt is buffered for this
 * time, before a next attempt to send the packet is started.
 *
 * If channel is idle the frame will be transmitted immediately.
 */
void CSMAMacLayer::handleUpperMsg(cPacket *msg)
{
    MacPkt *mac = encapsMsg(msg);

    // message has to be queued if another message is waiting to be send
    // or if we are already trying to send another message

    // the comparison with sendTime is necessary so that concurrently
    // arriving messages are handled sequentially. As soon as one
    // message arrived at simTime() is passed to lower layers all other
    // messages arriving at the same time will be buffered.
    if (timer->isScheduled() || radioState == RadioState::TRANSMIT || sendTime == simTime())
    {

        // if there is no queue the message will be deleted
        if (queueLength == 0)
        {
            EV << "New packet arrived though another is still waiting for being sent, "
                " and buffer size is zero. New packet is deleted.\n";
            // TODO: Signal this to upper layer!
            delete mac;
            return;
        }

        // the queue is not full yet so we can queue the message
        if (macQueue.length() < queueLength)
        {
            EV << "already transmitting, putting pkt into queue...\n";
            macQueue.insert(mac);
            return;
        }
        // queue is full, message has to be deleted
        else
        {
            EV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
            // TODO: Signal this to upper layer!!
            delete mac;
            return;
        }
    }

    // no message is scheduled for sending or currently being sent

    // check the radio status and transmit the message if the channel is
    // idle. Otherwise backoff for a random time and try again
    if (radioState == RadioState::IDLE)
    {
        EV << "CHANNEL IDLE, send...\n";
        sendDown(mac);
        //store the sending time
        sendTime = simTime();
    }
    else
    {
        timer->setContextPointer(mac);
        simtime_t randomTime = intuniform(0, 10) / 100.0;
        scheduleAt(simTime() + randomTime, timer);
        EV << "CHANNEL BUSY, I will try to retransmit at " << simTime() + randomTime << ".\n";
    }

}


/**
 * After the timer expires try to retransmit the message by calling
 * handleUpperMsg again.
 */
void CSMAMacLayer::handleSelfMsg(cMessage *msg)
{
    EV << "timer expired, calling handleUpperMsg again.. time: " << simTime() << endl;

    // timer expired for a buffered frame, try to send again
    handleUpperMsg((MacPkt *) msg->getContextPointer());
}


/**
 * Compare the address of this Host with the destination address in
 * frame. If they are equal or the frame is broadcast, we send this
 * frame to the upper layer. If not delete it.
 */
void CSMAMacLayer::handleLowerMsg(cPacket *msg)
{
    MacPkt *mac = check_and_cast<MacPkt *>(msg);

    //only foward to upper layer if message is for me or broadcast
    if (mac->getDestAddr() == myMacAddr || mac->getDestAddr().isBroadcast())
    {
        EV << "sending pkt to upper...\n";
        sendUp(mac);
    }
    else
    {
        EV << "packet not for me, deleting...\n";
        delete mac;
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */
MacPkt *CSMAMacLayer::encapsMsg(cPacket *netw)
{
    MacPkt *pkt = new MacPkt(netw->getName());
    pkt->setBitLength(272);

    // copy dest address from the Control Info attached to the network
    // mesage by the network layer
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(netw->removeControlInfo());

    EV << "ctrl removed, mac addr=" << ctrl->getDest() << endl;
    pkt->setDestAddr(ctrl->getDest());

    //delete the control info
    delete ctrl;

    //set the src address to own mac address
    pkt->setSrcAddr(myMacAddr);

    //encapsulate the network packet
    pkt->encapsulate(netw);
    EV << "pkt encapsulated\n";

    return pkt;
}

/**
 * Update the internal copy of the RadioState.
 *
 * If the RadioState switched from TRANSMIT to IDLE and there are still
 * messages in the queue, call handleUpperMsg in order to try to send
 * those now.
 */
void CSMAMacLayer::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method("receiveChangeNotification(%s, %s)", notificationCategoryName(category),
                 details?details->info().c_str() : "n/a");
    printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED)
    {
        // update the local copy of the radio state
        radioState = check_and_cast<RadioState *>(details)->getState();

        // NOTE: we may be invoked during INIT STAGE 1 too, when SnrEval notifies us
        // about the initial radio state. This function has to work correctly
        // even when called during initialization phase!

        // if the channel is idle now, the queue is not empty and no timer
        // is scheduled, this means that sending the previous message is
        // complete and the next one can be taken out of the queue
        if (radioState == RadioState::IDLE && !macQueue.empty() && !timer->isScheduled())
        {
            timer->setContextPointer(macQueue.pop());
            simtime_t randomTime = intuniform(0, 10) / 100.0;
            scheduleAt(simTime() + randomTime, timer);
            EV << "taking next pkt out of queue, schedule at " << simTime() + randomTime << endl;
        }
    }
}


