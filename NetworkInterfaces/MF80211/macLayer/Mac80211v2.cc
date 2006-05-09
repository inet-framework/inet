//
// Copyright (C) 2006 Andras Varga and Levente Mészáros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "Mac80211v2.h"
#include "Ieee802Ctrl_m.h"
#include "RadioState.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(Mac80211v2);

/****************************************************************
 * Construction functions.
 */
Mac80211v2::Mac80211v2()
{
    endSIFS = NULL;
    endDIFS = NULL;
    endBackoff = NULL;
    endTimeout = NULL;
    endReserve = NULL;
    radioStateChange = NULL;
}

Mac80211v2::~Mac80211v2()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(radioStateChange);
}

/****************************************************************
 * Initialization functions.
 */
void Mac80211v2::initialize(int stage)
{
    WirelessMacBase::initialize(stage);

    if (stage == 0)
    {
        EV << "Initializing stage 0\n";

        // initialize parameters
        maxQueueSize = par("maxQueueSize");
        bitrate = par("bitrate");
        const char *addressString = par("address");
        if (!strcmp(addressString, "auto")) {
            // assign automatic address
            address = MACAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(address.str().c_str());
        }
        else
            address.setAddress(addressString);

        // subscribe for the information of the carrier sense
        nb->subscribe(this, NF_RADIOSTATE_CHANGED);

        // initalize self messages
        endSIFS = new cMessage("SIFS");
        endDIFS = new cMessage("DIFS");
        endBackoff = new cMessage("Backoff");
        endTimeout = new cMessage("Timeout");
        endReserve = new cMessage("Reserve");
        radioStateChange = new cMessage("radioStateChange");

        // interface
        registerInterface();

        // state variables
        fsm.setName("Mac80211v2 State Machine");
        mode = DCF;
        radioState = RadioState::IDLE;
        retryCounter = 1;
        backoff = false;

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;

        // initialize watches
        WATCH(fsm);
        WATCH(radioState);
        WATCH(retryCounter);
        WATCH(backoff);

        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
}

void Mac80211v2::registerInterface()
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(parentModule()->fullName()) + 1];
    char *d = interfaceName;
    for (const char *s = parentModule()->fullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(1500);

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    InterfaceTable *ift = InterfaceTableAccess().get();
    ift->addInterface(e, this);
}

/****************************************************************
 * Message handling functions.
 */
void Mac80211v2::handleSelfMsg(cMessage *msg)
{
    EV << "received self message: " << msg << endl;
    handleWithFSM(msg);
}

void Mac80211v2::handleUpperMsg(cMessage *msg)
{
    if (msg->byteLength() > 2312)
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->byteLength());

    if (maxQueueSize && transmissionQueue.size() == maxQueueSize) {
        EV << "message " << msg << " received from higher layer but MAC queue is full, dropping message\n";
        delete msg;
        return;
    }

    Mac80211Pkt *frame = encapsulate(msg);
    EV << "message " << msg << " received from higher layer, destination = " << frame->getDestAddr() << ", encapsulated\n";
    transmissionQueue.push_back(frame);

    handleWithFSM(msg);
}

void Mac80211v2::handleLowerMsg(cMessage *msg)
{
    EV << "received message from lower layer: " << msg << endl;
    handleWithFSM(msg);
    delete msg;
}

void Mac80211v2::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();

    if (category == NF_RADIOSTATE_CHANGED) {
        radioState = check_and_cast<RadioState *>(details)->getState();
        EV << "radio state changed " << className() << ": " << details->info() << endl;

        handleWithFSM(radioStateChange);
    }
}

/**
 * Msg can be upper, lower, self or NULL (when radio state changes)
 */
void Mac80211v2::handleWithFSM(cMessage *msg)
{
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMsg(msg) && fsm.state() != IDLE) {
        EV << "deferring upper message transmission in " << fsm.stateName() << " state\n";
        return;
    }

    if (isRadioStateChange(msg) && fsm.state() != DEFER && fsm.state() != WAITDIFS && fsm.state() != BACKOFF) {
        EV << "ignoring radio state change in " << fsm.stateName() << " state\n";
        return;
    }

    Mac80211Pkt *frame = dynamic_cast<Mac80211Pkt*>(msg);
    logState();

    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Event_Transition(Rx-Data,
                                  mode == DCF && isLowerMsg(msg) && msg->kind() == DATA,
                                  WAITSIFS,
                sendUp(decapsulate(frame));
                numReceived++;
            );
            FSMA_Event_Transition(Rx-Broadcast,
                                  mode == DCF && isLowerMsg(msg) && msg->kind() == BROADCAST,
                                  IDLE,
                sendUp(decapsulate(frame));
                numReceivedBroadcast++;
            );
            FSMA_Event_Transition(Rx-RTS,
                                  mode == MACA && isForUs(msg) && isLowerMsg(msg) && msg->kind() == RTS,
                                  WAITSIFS,
            );
            FSMA_Event_Transition(Reserve,
                                  mode == MACA && !isForUs(msg) && isLowerMsg(msg) && (msg->kind() == DATA || msg->kind() == RTS || msg->kind() == CTS),
                                  RESERVE,
            );
            FSMA_Event_Transition(DataReady,
                                  isUpperMsg(msg),
                                  DEFER,
                resetStateVariables();
                generateBackoffPeriod();
            );
            FSMA_No_Event_Transition(Immediate-DataReady,
                                     !transmissionQueue.empty(),
                                     DEFER,
                resetStateVariables();
                generateBackoffPeriod();
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Event_Transition(WaitDIFS,
                                  isRadioStateChange(msg) && radioState == RadioState::IDLE,
                                  WAITDIFS,
            ;);
            FSMA_No_Event_Transition(Immediate-WaitDIFS,
                                     radioState == RadioState::IDLE || !backoff,
                                     WAITDIFS,
            ;);
        }
        FSMA_State(WAITDIFS)
        {
            FSMA_Enter(scheduleDIFSPeriod());
            FSMA_Event_Transition(DIFSover,
                                  msg == endDIFS,
                                  BACKOFF,
            ;);
            FSMA_Event_Transition(BUSY,
                                  isRadioStateChange(msg) && radioState != RadioState::IDLE,
                                  DEFER,
                backoff = true;
                cancelEvent(endDIFS);
            );
            FSMA_No_Event_Transition(Immediate-BUSY,
                                     radioState != RadioState::IDLE,
                                     DEFER,
                backoff = true;
                cancelEvent(endDIFS);
            );
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffPeriod());
            FSMA_Event_Transition(BBUSY,
                                  isRadioStateChange(msg) && radioState != RadioState::IDLE,
                                  DEFER,
                backoff = true;
                cancelEvent(endBackoff);
                backoffPeriod -= simTime() - endBackoff->sendingTime();
            );
            FSMA_Event_Transition(Tx-Broadcast,
                                  mode == DCF && msg == endBackoff && isBroadcast(transmissionQueue.front()),
                                  TRANSMITTING,
                sendBroadcastFrame(transmissionQueue.front());
                scheduleTimeoutPeriod(transmissionQueue.front());
            );
            FSMA_Event_Transition(Tx-Data,
                                  mode == DCF && msg == endBackoff,
                                  TRANSMITTING,
                sendDataFrame(transmissionQueue.front());
                scheduleTimeoutPeriod(transmissionQueue.front());
            );
            FSMA_No_Event_Transition(Immediate-Tx-Data,
                                     mode == DCF && radioState == RadioState::IDLE && !backoff,
                                     TRANSMITTING,
                cancelEvent(endBackoff);
                sendDataFrame(transmissionQueue.front());
                scheduleTimeoutPeriod(transmissionQueue.front());
            );
            FSMA_Event_Transition(Tx-RTS,
                                  mode == MACA && msg == endBackoff,
                                  TRANSMITTING,
                sendRTSFrame(transmissionQueue.front());
                scheduleRTSTimeoutPeriod();
            );
        }
        // TODO: Broadcasts are not acknowledged, so they have to go to IDLE (but when?)
        // TODO: Data transmission in MACA mode will not go to IDLE, (do we need the extra states for that?)
        FSMA_State(TRANSMITTING)
        {
            FSMA_Event_Transition(Rx-ACK,
                                  mode == DCF && isLowerMsg(msg) && msg->kind() == ACK,
                                  IDLE,
                cancelEvent(endTimeout);
                popTransmissionQueue();
                if (retryCounter == 1) numSentWithoutRetry++;
                numSent++;
                resetStateVariables();
            );
            FSMA_Event_Transition(Rx-CTS,
                                  mode == MACA && isLowerMsg(msg) && msg->kind() == CTS,
                                  TRANSMITTING,
                cancelEvent(endTimeout);
                sendDataFrame(transmissionQueue.front());
                scheduleTimeoutPeriod(transmissionQueue.front());
            );
            FSMA_Event_Transition(Tx-Broadcast,
                                  mode == DCF && msg == endTimeout && isBroadcast(transmissionQueue.front()),
                                  IDLE,
                popTransmissionQueue();
                resetStateVariables();
                numSentBroadcast++;
            ;);
            FSMA_Event_Transition(Tx-Failed,
                                  msg == endTimeout && retryCounter > RETRY_LIMIT,
                                  IDLE,
                popTransmissionQueue();
                resetStateVariables();
                numGivenUp++;
            ;);
            FSMA_Event_Transition(Timeout,
                                  msg == endTimeout,
                                  DEFER,
                backoff = true;
                retryCounter++;
                numRetry++;
                generateBackoffPeriod();
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Tx-ACK,
                                  mode == DCF && msg == endSIFS,
                                  IDLE,
                sendACKFrame((Mac80211Pkt*)endSIFS->contextPointer());
                delete (Mac80211Pkt*)endSIFS->contextPointer();
            );
            FSMA_Event_Transition(Tx-CTS,
                                  mode == MACA && msg == endSIFS,
                                  IDLE,
                sendCTSFrame((Mac80211Pkt*)endSIFS->contextPointer());
                delete (Mac80211Pkt*)endSIFS->contextPointer();
            );
        }
        FSMA_State(RESERVE)
        {
            FSMA_Enter(scheduleReservePeriod(frame));
            FSMA_Event_Transition(Release,
                                  mode == MACA && msg == endReserve,
                                  IDLE,
            );
            FSMA_Event_Transition(RReserve,
                                  mode == MACA && !isForUs(msg) && isLowerMsg(msg) && (msg->kind() == DATA || msg->kind() == RTS || msg->kind() == CTS),
                                  RESERVE,
                scheduleReservePeriod(frame);
            );
        }
    }

    logState();
}

/****************************************************************
 * Timer functions.
 */
void Mac80211v2::scheduleSIFSPeriod(Mac80211Pkt *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + SIFS, endSIFS);
}

void Mac80211v2::scheduleDIFSPeriod()
{
    EV << "scheduling DIFS period\n";
    scheduleAt(simTime() + DIFS, endDIFS);
}

void Mac80211v2::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void Mac80211v2::scheduleTimeoutPeriod(Mac80211Pkt *frameToSend)
{
    EV << "scheduling timeout period\n";
    if (isBroadcast(frameToSend))
        scheduleAt(simTime() + frameDuration(frameToSend->length()) + PROCESSING_TIMEOUT, endTimeout);
    else
        scheduleAt(simTime() + frameDuration(frameToSend->length()) + frameDuration(LENGTH_ACK) + PROCESSING_TIMEOUT, endTimeout);
}

void Mac80211v2::scheduleRTSTimeoutPeriod()
{
    scheduleAt(simTime() + frameDuration(LENGTH_RTS) + frameDuration(LENGTH_CTS) + PROCESSING_TIMEOUT, endTimeout);
}

void Mac80211v2::scheduleReservePeriod(Mac80211Pkt *frame)
{
    EV << "scheduling reserve period\n";

    simtime_t reserve = frame->getDuration();

    if (endReserve->isScheduled()) {
        reserve = max(reserve, endReserve->arrivalTime() - simTime());
        cancelEvent(endReserve);
    }

    scheduleAt(simTime() + reserve, endReserve);
}

/****************************************************************
 * Frame sender functions.
 */
void Mac80211v2::sendACKFrame(Mac80211Pkt *frameToAck)
{
    EV << "sending ACK frame\n";
    sendDown(buildACKFrame(frameToAck));
}

void Mac80211v2::sendDataFrame(Mac80211Pkt *frameToSend)
{
    EV << "sending Data frame\n";
    sendDown(buildDataFrame(frameToSend));
}

void Mac80211v2::sendBroadcastFrame(Mac80211Pkt *frameToSend)
{
    EV << "sending Broadcast frame\n";
    sendDown(buildBroadcastFrame(frameToSend));
}

void Mac80211v2::sendRTSFrame(Mac80211Pkt *frameToSend)
{
    EV << "sending RTS frame\n";
    sendDown(buildRTSFrame(frameToSend));
}

void Mac80211v2::sendCTSFrame(Mac80211Pkt *rtsFrame)
{
    EV << "sending CTS frame\n";
    sendDown(buildCTSFrame(rtsFrame));
}


/****************************************************************
 * Frame builder functions.
 */
Mac80211Pkt* Mac80211v2::encapsulate(cMessage *msg)
{
    Mac80211Pkt *frame = new Mac80211Pkt(msg->name());
    // headerLength, including final CRC-field
    frame->setLength(272);

    // copy dest address from the control info
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setDestAddr(ctrl->getDest());
    delete ctrl;

    // set the src address to our mac address (nic module id())
    frame->setSrcAddr(address);
    frame->encapsulate(msg);

    return frame;
}

cMessage* Mac80211v2::decapsulate(Mac80211Pkt *frame)
{
    return frame->decapsulate();
}

Mac80211Pkt *Mac80211v2::buildDataFrame(Mac80211Pkt *frameToSend)
{
    Mac80211Pkt *frame = (Mac80211Pkt *)frameToSend->dup();
    frame->setKind(DATA);
    frame->setSrcAddr(address);

    if (isBroadcast(frameToSend))
        frame->setDuration(0);
    else if (frameToSend->getFragmentation() == 0)
        frame->setDuration(SIFS + frameDuration(LENGTH_ACK));
    else
        frame->setDuration(3 * SIFS + 2 * frameDuration(LENGTH_ACK) + frameDuration(frameToSend->length()));

    return frame;
}

Mac80211Pkt *Mac80211v2::buildACKFrame(Mac80211Pkt *frameToACK)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-ack");
    frame->setKind(ACK);
    frame->setLength(LENGTH_ACK);

    frame->setSrcAddr(address);
    frame->setDestAddr(frameToACK->getSrcAddr());
    if (frameToACK->getFragmentation() == 0)
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - SIFS - frameDuration(LENGTH_ACK));

    return frame;
}

Mac80211Pkt *Mac80211v2::buildRTSFrame(Mac80211Pkt *frameToSend)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-rts");
    frame->setKind(RTS);
    frame->setLength(LENGTH_RTS);

    frame->setSrcAddr(address);
    frame->setDestAddr(frameToSend->getDestAddr());
    frame->setDuration(3 * SIFS + frameDuration(LENGTH_CTS) +
                       frameDuration(frameToSend->length()) +
                       frameDuration(LENGTH_ACK));

    return frame;
}

Mac80211Pkt *Mac80211v2::buildCTSFrame(Mac80211Pkt *rtsFrame)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-cts");
    frame->setKind(CTS);
    frame->setLength(LENGTH_CTS);

    frame->setSrcAddr(address);
    frame->setDestAddr(rtsFrame->getSrcAddr());
    frame->setDuration(rtsFrame->getDuration() - SIFS - frameDuration(LENGTH_CTS));

    return frame;
}

Mac80211Pkt *Mac80211v2::buildBroadcastFrame(Mac80211Pkt *frameToSend)
{
    Mac80211Pkt *frame = (Mac80211Pkt *)frameToSend->dup();
    frame->setKind(BROADCAST);
    return frame;
}

/****************************************************************
 * Helper functions.
 */
void Mac80211v2::setMode(Mode mode)
{
    if (mode == PCF)
        error("PCF mode not yet supported");

    this->mode = mode;
}

void Mac80211v2::resetStateVariables()
{
    backoff = false;
    backoffPeriod = 0;
    retryCounter = 1;
}

void Mac80211v2::generateBackoffPeriod()
{
    backoffPeriod = ((double) intrand(contentionWindow() + 1)) * ST;
    EV << "backoff period set to " << backoffPeriod << endl;
}

int Mac80211v2::contentionWindow()
{
    // if the next packet is broadcast then the contention window must be maximal
    if (isBroadcast(transmissionQueue.front()))
        return CW_MAX;// TODO: do we need a parameter here? broadcastBackoff;
    else
    {
        int cw = (CW_MIN + 1) * (1 << (retryCounter - 1)) - 1;

        if (cw > CW_MAX)
            cw = CW_MAX;

        return cw;
    }
}

bool Mac80211v2::isRadioStateChange(cMessage *msg)
{
    return msg == radioStateChange;
}

bool Mac80211v2::isBroadcast(cMessage *msg)
{
    return ((Mac80211Pkt *)msg)->getDestAddr().isBroadcast();
}

bool Mac80211v2::isForUs(cMessage *msg)
{
    return ((Mac80211Pkt *)msg)->getDestAddr() == address;
}

void Mac80211v2::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    Mac80211Pkt *temp = transmissionQueue.front();
    transmissionQueue.pop_front();
    delete temp;
}

double Mac80211v2::frameDuration(int bits)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

void Mac80211v2::logState()
{
    EV  << "state information: mode = " << modeName(mode) << ", state = " << fsm.stateName()
        << ", backoff = " << backoff << ", backoffPeriod = " << backoffPeriod
        << ", retryCounter = " << retryCounter << ", radioState = " << radioState << endl;
}

const char *Mac80211v2::frameTypeName(int type)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (type)
    {
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

const char *Mac80211v2::modeName(int mode)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (mode)
    {
        CASE(DCF);
        CASE(MACA);
        CASE(PCF);
    }
    return s;
#undef CASE
}
