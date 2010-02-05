/***************************************************************************
 * file:        Mac80211.cc
 *
 * author:      David Raguin / Marc L�bbers
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
 **************************************************************************/


#include "Mac80211.h"
#include "Ieee802Ctrl_m.h"
#include "RadioState.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"


Define_Module(Mac80211);


Mac80211::Mac80211()
{
    timeout = nav = contention = endTransmission = endSifs = NULL;
}

Mac80211::~Mac80211()
{
    cancelAndDelete(timeout);
    cancelAndDelete(nav);
    cancelAndDelete(contention);
    cancelAndDelete(endTransmission);
    cancelAndDelete(endSifs);
}

void Mac80211::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0)
    {
        EV << "Initializing stage 0\n";
        maxQueueSize = par("maxQueueSize");

        // subscribe for the information of the carrier sense
        nb->subscribe(this, NF_RADIOSTATE_CHANGED);

        // timers
        timeout = new cMessage("timeout", TIMEOUT);
        nav = new cMessage("NAV", NAV);
        contention = new cMessage("contention", CONTENTION);
        endTransmission = new cMessage("transmission", END_TRANSMISSION);
        endSifs = new cMessage("end SIFS", END_SIFS);

        BW = 0;
        state = IDLE;
        retryCounter = 1;
        broadcastBackoff = par("broadcastBackoff");
        rtsCts = par("rtsCts");
        bitrate = par("bitrate");
        delta = 1E-9; //XXX it's rather "epsilon", but this delta business looks a bit dodgy a solution anyway

        radioState = RadioState::IDLE; // until 1st receiveChangeNotification()

        EIFS = SIFS + DIFS + computePacketDuration(LENGTH_ACK);
        EV << "SIFS: " << SIFS << " DIFS: " << DIFS << " EIFS: " << EIFS << endl;

        // get registered in IInterfaceTable
        registerInterface();

        WATCH(state);
        WATCH(radioState);
    }
}


void Mac80211::registerInterface()
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
    e->setMtu(par("mtu"));            // FIXME

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().get();
    ift->addInterface(e, this);
}

void Mac80211::handleCommand(cMessage *msg)
{
    // no commands supported by Mac80211
    error("Non-packet message arrived from higher layer: (%s)%s", msg->getClassName(), msg->getName());
}

/**
 * This implementation does not support fragmentation, so it is tested
 * if the maximum length of the MPDU is exceeded.
 */
void Mac80211::handleUpperMsg(cPacket *msg)
{
    if (msg->getByteLength() > 2312)
        error("packet from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->getClassName(), msg->getName(), (int)(msg->getByteLength()));

    if (maxQueueSize && (int)fromUpperLayer.size() == maxQueueSize)
    {
        EV << "packet " << msg << " received from higher layer but MAC queue is full, deleting\n";
        delete msg;
        return;
    }

    Mac80211Pkt *mac = encapsMsg(msg);
    EV << "packet " << msg << " received from higher layer, dest=" << mac->getDestAddr() << ", encapsulated\n";

    fromUpperLayer.push_back(mac);
    // If the MAC is in the IDLE state, then start a new contention period
    if (state == IDLE && !endSifs->isScheduled())
    {
        tryWithoutBackoff = true;
        beginNewCycle();
    }
    else
    {
        EV << "enqueued, will be transmitted later\n";
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */
Mac80211Pkt *Mac80211::encapsMsg(cPacket *netw)
{
    Mac80211Pkt *pkt = new Mac80211Pkt(netw->getName());
    pkt->setBitLength(272);        // headerLength, including final CRC-field

    // copy dest address from the control info
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(netw->removeControlInfo());
    pkt->setDestAddr(ctrl->getDest());
    delete ctrl;

    // set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(myMacAddr);

    // encapsulate the network packet
    pkt->encapsulate(netw);

    return pkt;
}

void Mac80211::decapsulateAndSendUp(Mac80211Pkt *frame)
{
    cPacket *msg = frame->decapsulate();
    // FIXME TBD set control info
    delete frame;
    sendUp(msg);
}

/**
 *  Handle all messages from lower layer. Checks the destination MAC
 *  adress of the packet. Then calls one of the three functions :
 *  handleMsgNotForMe(), handleBroadcastMsg(), or handleMsgForMe().
 *  Called by handleMessage().
 */
void Mac80211::handleLowerMsg(cPacket *msg)
{
    Mac80211Pkt *af = check_and_cast<Mac80211Pkt *>(msg);

    // end of the reception
    EV << "frame " << af << " received, kind = " << pktTypeName(af->getKind()) << "\n";

    switch (af->getKind())
    {
    case COLLISION: // packet lost or bit error
        delete af;
        if (state == CONTEND)
            beginNewCycle();
        break;

    case BITERROR:
        handleMsgNotForMe(af);
        break;

    case BROADCAST: // broadcast packet
        handleBroadcastMsg(af);
        break;

    default: // other packet
        if (af->getDestAddr() == myMacAddr)  // FIXME verify broadcast dest addr works!
            handleMsgForMe(af);
        else
            handleMsgNotForMe(af);
    }
}

/**
 * handle timers
 */
void Mac80211::handleSelfMsg(cMessage * msg)
{
    EV << "processing self message with type = " << timerTypeName(msg->getKind()) << endl;

    switch (msg->getKind())
    {
    case END_SIFS:
        handleEndSifsTimer();   // noch zu betrachten
        break;

    case END_TRANSMISSION:
        handleEndTransmissionTimer();   // noch zu betrachten
        break;

    case CONTENTION:
        handleEndContentionTimer();
        break;

        // the MAC was waiting for a CTS, a DATA, or an ACK packet but the timer has expired.
    case TIMEOUT:
        handleTimeoutTimer();   // noch zu betrachten..
        break;

        // the MAC was waiting because an other communication had won the channel. This communication is now over
    case NAV:
        handleNavTimer();       // noch zu betrachten...
        break;

    default:
        error("unknown timer type");
    }
}


/**
 *  Handle all ACKs,RTS, CTS, or DATA not for the node. If RTS/CTS
 *  is used the node must stay quiet until the current handshake
 *  between the two communicating nodes is over.  This is done by
 *  scheduling the timer message nav (Network Allocation Vector).
 *  Without RTS/CTS a new contention is started. If an error
 *  occured the node must defer for EIFS.  Called by
 *  handleLowerMsg()
 */
void Mac80211::handleMsgNotForMe(Mac80211Pkt *af)
{
    EV << "handle msg not for me\n";

    // if this packet  can not be correctly read
    if (af->getKind() == BITERROR)
        af->setDuration(EIFS);

    // if the duration of the packet is null, then do nothing (to avoid
    // the unuseful scheduling of a self message)
    if (af->getDuration() != 0)
    {

        // the node is already deferring
        if (state == QUIET)
        {
            // the current value of the NAV is not sufficient
            if (nav->getArrivalTime() < simTime() + af->getDuration())
            {
                cancelEvent(nav);
                scheduleAt(simTime() + af->getDuration(), nav);
                EV << "NAV timer started for: " << af->getDuration() << " State QUIET\n";
            }
        }

        // other states
        else
        {
            // if the MAC wait for another frame, it can delete its time out
            // (exchange is aborted)
            if (timeout->isScheduled())
                cancelEvent(timeout);

            // is state == WFCTS or WFACK, the data transfer has failed ...

            // the node must defer for the time of the transmission
            scheduleAt(simTime() + af->getDuration(), nav);
            EV << "NAV timer started, not QUIET: " << af->getDuration() << endl;
            setState(QUIET);

        }
    }
    if (!rtsCts)
    {                           // todo: Nachgucken: was passiert bei Error ohne rtsCts!
        if (state == CONTEND)
        {
            if (af->getKind() == BITERROR)
            {
                if (contention->isScheduled())
                    cancelEvent(contention);
                scheduleAt(simTime() + computeBackoff() + EIFS, contention);
            }
            else
                beginNewCycle();
        }
    }
    delete af;
}


/**
 *  Handle a packet for the node. The result of this reception is
 *  a function of the type of the received message (RTS,CTS,DATA,
 *  or ACK), and of the current state of the MAC (WFDATA, CONTEND,
 *  IDLE, WFCTS, or WFACK). Called by handleLowerMsg()
 */
void Mac80211::handleMsgForMe(Mac80211Pkt *af)
{
    EV << "handle msg for me in state = " << stateName(state) << " with type = " << pktTypeName(af->getKind()) << "\n";

    switch (state)
    {
    case IDLE:     // waiting for the end of the contention period
    case CONTEND:  // or waiting for RTS

        // RTS or DATA accepted
        if (af->getKind() == RTS)
            handleRTSframe(af);
        else if (af->getKind() == DATA)
            handleDATAframe(af);
        else
            // TODO: what if a late ACK has arrived?
            EV << "in handleMsgForMe() IDLE/CONTEND, strange message, darf das?\n";
        break;

    case WFDATA:  // waiting for DATA

        if (af->getKind() == DATA)
            handleDATAframe(af);
        else
            EV << "in handleMsgForMe() WFDATA, strange message, darf das?\n";
        break;

    case WFACK:  // waiting for ACK

        if (af->getKind() == ACK)
            handleACKframe(af);
        else
            EV << "in handleMsgForMe() WFACK, strange message, darf das?\n";
        delete af;
        break;

    case WFCTS:  // The MAC is waiting for CTS

        if (af->getKind() == CTS)
            handleCTSframe(af);
        else
            EV << "in handleMsgForMe() WFCTS, strange message, darf das?\n";
        break;


    case QUIET: // the node is currently deferring.

        // cannot handle any packet with its MAC adress
        delete af;
        break;

    case BUSY: // currently transmitting an ACK or a BROADCAST packet
        error("logic error: node is currently transmitting, can not receive "
              "(does the physical layer do its job correctly?)");
        break;

    default:
        error("unknown state %d", state);
    }
}

/**
 *  Handle aframe wich is expected to be an RTS. Called by
 *  HandleMsgForMe()
 */
void Mac80211::handleRTSframe(Mac80211Pkt * af)
{
    // wait a short interframe space
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle a frame which expected to be a DATA frame. Called by
 *  HandleMsgForMe()
 */
void Mac80211::handleDATAframe(Mac80211Pkt * af)
{
    if (rtsCts)
        cancelEvent(timeout);  // cancel time-out event

    // make a copy
    Mac80211Pkt *copy = (Mac80211Pkt *) af->dup();

    // pass the packet to the upper layer
    decapsulateAndSendUp(af);

    // wait a short interframe space
    endSifs->setContextPointer(copy);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle a frame which is expected to be an ACK.Called by
 *  HandleMsgForMe(MAcawFrame* af)
 */
void Mac80211::handleACKframe(Mac80211Pkt * af)
{
    EV << "handling Ack frame\n";

    // cancel time-out event
    cancelEvent(timeout);

    // the transmission is acknowledged : initialize long_retry_counter
    retryCounter = 1;

    // removes the acknowledged packet from the queue
    Mac80211Pkt *temp = fromUpperLayer.front();
    fromUpperLayer.pop_front();
    delete temp;

    // if thre's a packet to send and if the channel is free then start a new contention period
    beginNewCycle();
}


/**
 *  Handle a CTS frame. Called by HandleMsgForMe(Mac80211Pkt* af)
 */
void Mac80211::handleCTSframe(Mac80211Pkt * af)
{
    // cancel time-out event
    cancelEvent(timeout);

    // wait a short interframe space
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle a broadcast packet. This packet is simply passed to the
 *  upper layer. No acknowledgement is needed.  Called by
 *  handleLowerMsg(Mac80211Pkt *af)
 */
void Mac80211::handleBroadcastMsg(Mac80211Pkt *af)
{
    EV << "handle broadcast\n";
    if (state == BUSY)
        error("logic error: node is currently transmitting, can not receive "
              "(does the physical layer do its job correctly?)");

    decapsulateAndSendUp(af);
    if (state == CONTEND)
        beginNewCycle();
}


/**
 *  The node has won the contention, and is now allowed to send an
 *  RTS/DATA or Broadcast packet. The backoff value is deleted and
 *  will be newly computed in the next contention period
 */
void Mac80211::handleEndContentionTimer()
{
    EV << "end contention period\n";

    if (state != CONTEND)
        error("logic error: expiration of the contention timer outside of CONTEND state, should not happen");

    // the node has won the channel, the backoff window is deleted and
    // will be new calculated in the next contention period
    BW = 0;
    // unicast packet
    if (!nextIsBroadcast)
    {
        if (rtsCts)
        {
            // send a RTS
            sendRTSframe();
            setState(WFCTS);
        }
        else
        {
            sendDATAframe();
            setState(WFACK);
        }

        // broadcast packet
    }
    else
    {
        sendBROADCASTframe();

        // removes the packet from the queue without waiting for an acknowledgement
        Mac80211Pkt *temp = fromUpperLayer.front();
        fromUpperLayer.pop_front();
        delete(temp);
    }
}

/**
 *  Handle the NAV timer (end of a defering period). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211::handleNavTimer()
{
    if (state != QUIET)
        error("logic error: expiration of the NAV timer outside of the state QUIET, should not happen");

    // if there's a packet to send and if the channel is free, then start a new contention period
    beginNewCycle();
}


/**
 *  Handle the time out timer. Called by handleTimer(cMessage* msg)
 */
void Mac80211::handleTimeoutTimer()
{
    // if (state == WFCTS || state == WFACK)testMaxAttempts();

    // if there's a packet to send and if the channel is free then
    // start a new contention period
    if (state != QUIET)
        beginNewCycle();
}


/**
 *  Handle the end sifs timer. Then sends a CTS, a DATA, or an ACK
 *  frame
 */
void Mac80211::handleEndSifsTimer()
{
    Mac80211Pkt *frame = (Mac80211Pkt *) endSifs->getContextPointer();

    switch (frame->getKind())
    {
    case RTS:
        sendCTSframe(frame);
        break;
    case CTS:
        sendDATAframe();
        break;
    case DATA:
        sendACKframe(frame);
        break;
    default:
        error("logic error: end sifs timer when previously received packet is not RTS/CTS/DATA");
    }

    // don't need previous frame any more
    delete frame;
}

/**
 *  Handle the end of transmission timer (end of the transmission
 *  of an ACK or a broadcast packet). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211::handleEndTransmissionTimer()
{
    EV << "transmission of ACK/BROADCAST is over\n";
    if (state != BUSY)
        error("logic error: expiration of the end transmission timer outside the BUSY state, should not happen");

    // if there's a packet to send and if the channel is free, then start a new contention period
    beginNewCycle();
}


/**
 *  Send a DATA frame. Called by HandleEndSifsTimer() or
 *  handleEndContentionTimer()
 */
void Mac80211::sendDATAframe()
{
    EV << "sending data frame\n";

    // schedule time out
    scheduleAt(simTime() + computeTimeout(DATA, 0), timeout);

    if (!rtsCts)
        // retryCounter incremented
        retryCounter++;

    // send DATA frame
    sendDown(buildDATAframe());

    // update state and display
    setState(WFACK);
}


/**
 *  Send an ACK frame.Called by HandleEndSifsTimer()
 */
void Mac80211::sendACKframe(Mac80211Pkt * af)
{
    // the MAC must wait the end of the transmission before beginning an
    // other contention period
    scheduleAt(simTime() + computePacketDuration(LENGTH_ACK) + delta, endTransmission);

    // send ACK frame
    sendDown(buildACKframe(af));
    EV << "sent ACK frame!\n";

    // update state and display
    setState(BUSY);
}


/**
 *  Send a RTS frame.Called by handleContentionTimer()
 */
void Mac80211::sendRTSframe()
{
    // schedule time-out
    scheduleAt(simTime() + computeTimeout(RTS, 0), timeout);

    // long_retry_counter incremented
    retryCounter++;

    // send RTS frame
    sendDown(buildRTSframe());

    // update state and display
    setState(WFCTS);
}


/**
 *  Send a CTS frame.Called by HandleEndSifsTimer()
 */
void Mac80211::sendCTSframe(Mac80211Pkt * af)
{
    // schedule time-out
    scheduleAt(simTime() + computeTimeout(CTS, af->getDuration()), timeout);

    // send CTS frame
    sendDown(buildCTSframe(af));

    // update state and display
    setState(WFDATA);
}

/**
 *  Send a BROADCAST frame.Called by handleContentionTimer()
 */
void Mac80211::sendBROADCASTframe()
{
    // the MAC must wait the end of the transmission before beginning any
    // other contention period
    scheduleAt(simTime() + computePacketDuration(fromUpperLayer.front()->getBitLength()), endTransmission);
    // send ACK frame
    sendDown(buildBROADCASTframe());

    // update state and display
    setState(BUSY);
}


/**
 *  Build a DATA frame. Called by sendDATAframe()
 */
Mac80211Pkt *Mac80211::buildDATAframe()
{
    // build a copy of the frame in front of the queue
    Mac80211Pkt *frame = (Mac80211Pkt *) (fromUpperLayer.front())->dup();
    frame->setSrcAddr(myMacAddr);
    frame->setKind(DATA);
    if (rtsCts)
        frame->setDuration(SIFS + computePacketDuration(LENGTH_ACK));
    else
        frame->setDuration(0);

    return frame;
}


/**
 *  Build an ACK frame. Called by sendACKframe()
 */
Mac80211Pkt *Mac80211::buildACKframe(Mac80211Pkt * af)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-ack");
    frame->setKind(ACK);
    frame->setBitLength(LENGTH_ACK);

    // the dest address must be the src adress of the RTS or the DATA
    // packet received. The src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(0);

    return frame;
}


/**
 *  Build a RTS frame. Called by sendRTSframe()
 */
Mac80211Pkt *Mac80211::buildRTSframe()
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-rts");
    frame->setKind(RTS);
    frame->setBitLength(LENGTH_RTS);

    // the src adress and dest address are copied in the frame in the queue (frame to be sent)
    frame->setSrcAddr(((Mac80211Pkt *) fromUpperLayer.front())->getSrcAddr());
    frame->setDestAddr(((Mac80211Pkt *) fromUpperLayer.front())->getDestAddr());
    frame->setDuration(3 * SIFS + computePacketDuration(LENGTH_CTS) +
                       computePacketDuration(fromUpperLayer.front()->getBitLength()) +
                       computePacketDuration(LENGTH_ACK));

    return frame;
}

/**
 *  Build a CTS frame. Called by sendCTSframe()
 */
Mac80211Pkt *Mac80211::buildCTSframe(Mac80211Pkt * af)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-cts");
    frame->setKind(CTS);
    frame->setBitLength(LENGTH_CTS);

    // the dest adress must be the src adress of the RTS received. The
    // src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(af->getDuration() - SIFS - computePacketDuration(LENGTH_CTS));

    return frame;
}

/**
 *  Build a BROADCAST frame. Called sendBROADCASTframe()
 */
Mac80211Pkt *Mac80211::buildBROADCASTframe()
{
    // send a copy of the frame in front of the queue
    Mac80211Pkt *frame = (Mac80211Pkt *) (fromUpperLayer.front())->dup();
    frame->setKind(BROADCAST);
    return frame;
}


/**
 *  Start a new contention period if the channel is free and if
 *  there's a packet to send.  Called at the end of a deferring
 *  period, a busy period, or after a failure. Called by the
 *  HandleMsgForMe(), HandleTimer() HandleUpperMsg(), and, without
 *  RTS/CTS, by handleMsgNotForMe().
 */
void Mac80211::beginNewCycle()
{
    EV << "beginning new contention cycle\n";

    // before trying to send one more time a packet, test if the
    // maximum retry limit is reached. If it is the case, then
    // delete the packet and send the next packet.
    testMaxAttempts();

    if (!fromUpperLayer.empty())
    {

        // look if the next packet is unicast or broadcast
        nextIsBroadcast = (((Mac80211Pkt *) fromUpperLayer.front())->getDestAddr().isBroadcast());

        // print("next is broadcast = "<<nextIsBroadcast);

        // if the channel is free then wait a random time and transmit
        if (radioState == RadioState::IDLE)
        {
            // if channel is idle AND I was not the last one that transmitted
            // data: no backoff
            if (tryWithoutBackoff)
            {
                EV << "trying to send without backoff...\n";
                scheduleAt(simTime() + DIFS, contention);
            }
            else
            {
                // backoff!
                scheduleAt(simTime() + computeBackoff() + DIFS, contention);
            }
        }
        tryWithoutBackoff = false;

        // else wait until the channel gets free; the state is now contend
        setState(CONTEND);
    }
    else
    {
        tryWithoutBackoff = false;
        setState(IDLE);
    }
}

/**
 * Compute the backoff value.
 */
simtime_t Mac80211::computeBackoff()
{
    // the MAC has won the previous contention. We have to compute a new
    // backoff window
    if (BW == 0) {
        int CW = computeContentionWindow();
        EV << "generating backoff for CW: " << CW << endl;
        BW = intrand(CW + 1) * ST;
    }
    // CW is the contention window (see the function). ST is the
    // slot time.  else we take the old value of BW, in order to give a
    // bigger priority to a node which has lost a previous contention
    // period.
    EV << "backing off for: " << BW + DIFS << endl;
    return BW;
}

/**
 *  Compute the contention window with the binary backoff
 *  algorithm.  Use the variable counter (attempts to transmit
 *  packet), the constant values CWmax (contention window maximum)
 *  and m (parameter for the initial backoff window, usally m=7).
 *  Called by computeBackoff()
 */
int Mac80211::computeContentionWindow()
{
    // the next packet is an unicast packet
    if (!nextIsBroadcast)
    {
        int cw = (CW_MIN + 1) * (1 << (retryCounter - 1)) - 1;
        // return the calculated value or CWmax if the maximal value is reached
        if (cw <= CW_MAX)
            return cw;
        else
            return CW_MAX;
    }

    // the next packet is broadcast : the contention window must be maximal
    else
        return broadcastBackoff;
}



/**
 *  Test if the maximal retry limit is reached, and delete the
 *  frame to send in this case.
 */
void Mac80211::testMaxAttempts()
{
    if (retryCounter > RETRY_LIMIT)
    {
        // initialize counter
        retryCounter = 1;
        // reportLost(fromUpperLayer.front());

        // delete the frame to transmit
        Mac80211Pkt *temp = fromUpperLayer.front();
        fromUpperLayer.pop_front();
        delete(temp);
    }
}

/**
 * Handle change nofitications. In this layer it is usually
 * information about the radio channel, i.e. if it is IDLE etc.
 */
void Mac80211::receiveChangeNotification(int category, const cPolymorphic *details)
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

        EV << "** Radio state update in " << getClassName() << ": " << details->info()
           << " (at T=" << simTime() << ")\n";

        // beginning of a reception
        if (radioState == RadioState::RECV)
        {
            // if there's a contention period
            if (contention->isScheduled())
            {
                // update the backoff window in order to give higher priority in
                // the next battle
                if (simTime() - contention->getSendingTime() >= DIFS)
                {
                    BW = contention->getArrivalTime() - simTime();
                    EV << "Backoff window made smaller, new BW: " << BW << endl;
                }
                cancelEvent(contention);
            }

            // if there's a SIFS period
            if (endSifs->isScheduled())
            {
                // delete the previously received frame
                delete (Mac80211Pkt *)endSifs->getContextPointer();

                // cancel the next transmission
                cancelEvent(endSifs);

                // state in now IDLE or CONTEND
                if (fromUpperLayer.empty())
                    setState(IDLE);
                else
                    setState(CONTEND);
            }
        }
    }
}


/**
 *  Return a time-out value for a type of frame. Called by
 *  SendRTSframe, sendCTSframe, etc.
 */
simtime_t Mac80211::computeTimeout(_802_11frameType type, simtime_t last_frame_duration)
{
    simtime_t time_out = 0;
    switch (type)
    {
    case RTS:
        time_out = SIFS + computePacketDuration(LENGTH_RTS) + computePacketDuration(LENGTH_CTS) + delta;
        break;
    case CTS:
        time_out = last_frame_duration - computePacketDuration(LENGTH_ACK) - 2 * SIFS + delta;
        break;
    case DATA:
        time_out =
            SIFS + computePacketDuration(fromUpperLayer.front()->getBitLength()) + computePacketDuration(LENGTH_ACK) +
            delta + 0.1;
        //XXX: I have added some time here, because propagation delay of AirFrames caused problems
        // the timeout periods should be carefully revised with special care for the deltas?! --Levy
        break;
    default:
        EV << "Unused frame type was given when calling computeTimeout(), this should not happen!\n";
    }
    return time_out;
}

/**
 * Computes the duration of the transmission of a frame over the
 * physical channel. 'bits' should be the total length of the MAC
 * packet in bits.
 */
simtime_t Mac80211::computePacketDuration(int bits)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

const char *Mac80211::stateName(State state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (state)
    {
        CASE(WFDATA);
        CASE(QUIET);
        CASE(IDLE);
        CASE(CONTEND);
        CASE(WFCTS);
        CASE(WFACK);
        CASE(BUSY);
    }
    return s;
#undef CASE
}

const char *Mac80211::timerTypeName(int type)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (type)
    {
        CASE(TIMEOUT);
        CASE(NAV);
        CASE(CONTENTION);
        CASE(END_TRANSMISSION);
        CASE(END_SIFS);
    }
    return s;
#undef CASE
}

const char *Mac80211::pktTypeName(int type)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (type)
    {
        CASE(TIMEOUT);
        CASE(DATA);
        CASE(BROADCAST);
        CASE(RTS);
        CASE(CTS);
        CASE(ACK);
        CASE(ACKRTS);
        CASE(BEGIN_RECEPTION);
        CASE(BITERROR);
        CASE(COLLISION);
    }
    return s;
#undef CASE
}

void Mac80211::setState(State newState)
{
    if (state==newState)
        EV << "staying in state " << stateName(state) << "\n";
    else
        EV << "state " << stateName(state) << " --> " << stateName(newState) << "\n";
    state = newState;
}


