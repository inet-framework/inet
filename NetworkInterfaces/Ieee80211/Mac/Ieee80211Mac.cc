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

#include "Ieee80211Mac.h"
#include "RadioState.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(Ieee80211Mac);

/****************************************************************
 * Construction functions.
 */
Ieee80211Mac::Ieee80211Mac()
{
    endSIFS = NULL;
    endDIFS = NULL;
    endBackoff = NULL;
    endTimeout = NULL;
    endReserve = NULL;
    mediumStateChange = NULL;
}

Ieee80211Mac::~Ieee80211Mac()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(mediumStateChange);
}

/****************************************************************
 * Initialization functions.
 */
void Ieee80211Mac::initialize(int stage)
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
        mediumStateChange = new cMessage("MediumStateChange");

        // interface
        registerInterface();

        // obtain pointer to external queue
        initializeQueueModule();

        // state variables
        fsm.setName("Ieee80211Mac State Machine");
        mode = DCF;
        sequenceNumber = 0;
        radioState = RadioState::IDLE;
        retryCounter = 1;
        backoff = false;
        lastReceiveFailed = false;
        nav = false;

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numCollision = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;
        stateVector.setName("State");
        radioStateVector.setName("RadioState");

        // initialize watches
        WATCH(fsm);
        WATCH(radioState);
        WATCH(retryCounter);
        WATCH(backoff);
        WATCH(nav);

        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numCollision);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
}

void Ieee80211Mac::registerInterface()
{
    InterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

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
    ift->addInterface(e, this);
}

void Ieee80211Mac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = parentModule()->submodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV << "Requesting first frame from queue module\n";
        queueModule->requestPacket();
    }
}

/****************************************************************
 * Message handling functions.
 */
void Ieee80211Mac::handleSelfMsg(cMessage *msg)
{
    EV << "received self message: " << msg << endl;

    if (msg == endReserve)
        nav = false;

    handleWithFSM(msg);
}

void Ieee80211Mac::handleUpperMsg(cMessage *msg)
{
    if (maxQueueSize && transmissionQueue.size() == maxQueueSize)
    {
        EV << "message " << msg << " received from higher layer but MAC queue is full, dropping message\n";
        delete msg;
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->byteLength() > fragmentationThreshold)
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->byteLength());
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!

    transmissionQueue.push_back(frame);

    handleWithFSM(frame);
}

void Ieee80211Mac::handleLowerMsg(cMessage *msg)
{
    EV << "received message from lower layer: " << msg << endl;
    if (!dynamic_cast<Ieee80211Frame *>(msg))
        error("message from physical layer (%s)%s is not a subclass of Ieee80211Frame",
              msg->className(), msg->name());

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->owner() == this)
        delete msg;
}

void Ieee80211Mac::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();

    if (category == NF_RADIOSTATE_CHANGED)
    {
        RadioState::States newRadioState = check_and_cast<RadioState *>(details)->getState();
        EV << "radio state changed " << className() << ": " << details->info() << endl;

        // FIXME: double recording, because there's no sample hold in the gui
        radioStateVector.record(radioState);
        radioStateVector.record(newRadioState);

        radioState = newRadioState;

        handleWithFSM(mediumStateChange);
    }
}

/**
 * Msg can be upper, lower, self or NULL (when radio state changes)
 */
void Ieee80211Mac::handleWithFSM(cMessage *msg)
{
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMsg(msg) && fsm.state() != IDLE)
    {
        EV << "deferring upper message transmission in " << fsm.stateName() << " state\n";
        return;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame*>(msg);
    int frameType = frame ? frame->getType() : -1;
    int msgKind = msg->kind();
    logState();
    stateVector.record(fsm.state());

    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Event_Transition(Data-Ready,
                                  isUpperMsg(msg),
                                  DEFER,
                resetStateVariables();
                generateBackoffPeriod();
            );
            FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     !transmissionQueue.empty(),
                                     DEFER,
                resetStateVariables();
                generateBackoffPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVING,
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Event_Transition(Wait-DIFS,
                                  isMediumStateChange(msg) && isMediumFree(),
                                  WAITDIFS,
                scheduleDIFSPeriod();
            );
            FSMA_No_Event_Transition(Immediate-Wait-DIFS,
                                     isMediumFree() || !backoff,
                                     WAITDIFS,
                scheduleDIFSPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVING,
            );
        }
        FSMA_State(WAITDIFS)
        {
            FSMA_Event_Transition(Immediate-Transmit-Data,
                                     msg == endDIFS && !backoff,
                                     TRANSMITTING,
                sendDataFrame(currentTransmission());
                scheduleTimeoutPeriod(currentTransmission());
            );
            FSMA_Event_Transition(DIFS-Over,
                                  msg == endDIFS,
                                  BACKOFF,
                ASSERT(backoff);
                scheduleBackoffPeriod();
            );
            FSMA_Event_Transition(Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                backoff = true;
                cancelDIFSPeriod();
            );
            FSMA_No_Event_Transition(Immediate-Busy,
                                     !isMediumFree(),
                                     DEFER,
                backoff = true;
                cancelDIFSPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVING,
                cancelDIFSPeriod();
            );
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Event_Transition(Backoff-Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                cancelBackoffPeriod();
                decreaseBackoffPeriod();
            );
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endBackoff && isBroadcast(currentTransmission()),
                                  TRANSMITTING,
                sendBroadcastFrame(currentTransmission());
                scheduleTimeoutPeriod(currentTransmission());
            );
            FSMA_Event_Transition(Transmit-Data,
                                  mode == DCF && msg == endBackoff,
                                  TRANSMITTING,
                sendDataFrame(currentTransmission());
                scheduleTimeoutPeriod(currentTransmission());
            );
            FSMA_Event_Transition(Transmit-RTS,
                                  mode == MACA && msg == endBackoff,
                                  TRANSMITTING,
                sendRTSFrame(currentTransmission());
                scheduleRTSTimeoutPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVING,
                cancelBackoffPeriod();
            );
        }
        // TODO: revise whether RTS, CTS needs separate states
        FSMA_State(TRANSMITTING)
        {
            FSMA_Event_Transition(Receive-ACK,
                                  isLowerMsg(msg) && frameType == ST_ACK,
                                  IDLE,
                cancelTimeoutPeriod();
                popTransmissionQueue();
                if (retryCounter == 1) numSentWithoutRetry++;
                numSent++;
                resetStateVariables();
            );
//            FSMA_Event_Transition(Receive-CTS,
//                                  mode == MACA && isLowerMsg(msg) && frameType == ST_CTS,
//                                  TRANSMITTING,
//                cancelTimeoutPeriod();
//                sendDataFrame(currentTransmission());
//                scheduleTimeoutPeriod(currentTransmission());
//            );
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endTimeout && isBroadcast(currentTransmission()),
                                  IDLE,
                popTransmissionQueue();
                resetStateVariables();
                numSentBroadcast++;
            ;);
            FSMA_Event_Transition(Transmit-Failed,
                                  msg == endTimeout && retryCounter > RETRY_LIMIT,
                                  IDLE,
                popTransmissionQueue();
                resetStateVariables();
                numGivenUp++;
            ;);
            FSMA_Event_Transition(Transmit-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                currentTransmission()->setRetry(true);
                retryCounter++;
                numRetry++;
                backoff = true;
                generateBackoffPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMsg(msg),
                                  RECEIVING,
                cancelTimeoutPeriod();
            );
        }
        FSMA_State(RECEIVING)
        {
            FSMA_No_Event_Transition(Receive-Broadcast,
                                     isLowerMsg(msg) && isBroadcast(frame) && frameType == ST_DATA,
                                     IDLE,
                lastReceiveFailed = false;
                sendUp(frame);
                numReceivedBroadcast++;
            );
            FSMA_No_Event_Transition(Receive-Data,
                                     isLowerMsg(msg) && isForUs(frame) && frameType == ST_DATA,
                                     WAITSIFS,
                lastReceiveFailed = false;
                sendUp(frame);
                numReceived++;
            );
            FSMA_No_Event_Transition(Receive-RTS,
                                     mode == MACA && isLowerMsg(msg) && isForUs(frame) && frameType == ST_RTS, 
                                     WAITSIFS,
            );
            FSMA_No_Event_Transition(Receive-Error,
                                     isLowerMsg(msg) && (msgKind == COLLISION || msgKind == BITERROR),
                                     IDLE,
                EV << "received frame contains bit errors or collision, next wait period is EIFSn"; 
                lastReceiveFailed = true;
                numCollision++;
            );
            FSMA_No_Event_Transition(Receive-Other,
                                     isLowerMsg(msg) && !isForUs(frame),
                                     IDLE,
                if (frame->getDuration() != 0)
                {                                                                                    
                    EV << "received frame with network allocationn";
                    scheduleReservePeriod(frame);
                }
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-ACK,
                                  mode == DCF && msg == endSIFS,
                                  IDLE,
                sendACKFrameOnEndSIFS();
            );
            FSMA_Event_Transition(Transmit-CTS,
                                  mode == MACA && msg == endSIFS,
                                  IDLE,
                sendCTSFrameOnEndSIFS();
            );
        }
    }

    logState();
    stateVector.record(fsm.state());
}

/****************************************************************
 * Timing functions.
 */
simtime_t Ieee80211Mac::SIFSPeriod()
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    return SIFS;
}

simtime_t Ieee80211Mac::SlotPeriod()
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    return ST;
}

simtime_t Ieee80211Mac::PIFSPeriod()
{
    return SIFSPeriod() + SlotPeriod();
}

simtime_t Ieee80211Mac::DIFSPeriod()
{
    return SIFSPeriod() + 2 * SlotPeriod();
}

simtime_t Ieee80211Mac::EIFSPeriod()
{
// FIXME:   return SIFSPeriod() + DIFSPeriod() + (8 * ACKSize + aPreambleLength + aPLCPHeaderLength) / lowestDatarate;
    return SIFSPeriod() + DIFSPeriod() + (8 * LENGTH_ACK) / 1E+6;
}

simtime_t Ieee80211Mac::BackoffPeriod(Ieee80211Frame *msg, int r)
{
    int cw;

    // FIXME: if the next packet is broadcast then the contention window must be maximal?
    // FIXME: I could not verify this in the spec
    // TODO: do we need a parameter here such as broadcastBackoffCW?
    if (isBroadcast(msg))
        cw = CW_MAX;
    else
    {
        cw = (CW_MIN + 1) * (1 << (r - 1)) - 1;

        if (cw > CW_MAX)
            cw = CW_MAX;
    }

    return ((double)intrand(cw + 1)) * SlotPeriod();
}

/****************************************************************
 * Timer functions.
 */
void Ieee80211Mac::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + SIFSPeriod(), endSIFS);
}

void Ieee80211Mac::scheduleDIFSPeriod()
{
    if (lastReceiveFailed)
    {
        EV << "receiption of last frame failed, scheduling EIFS period\n";
        scheduleAt(simTime() + EIFSPeriod(), endDIFS);
    }
    else
    {
        EV << "scheduling DIFS period\n";
        scheduleAt(simTime() + DIFSPeriod(), endDIFS);
    }
}

void Ieee80211Mac::cancelDIFSPeriod()
{
    EV << "cancelling DIFS period\n";
    cancelEvent(endDIFS);
}

void Ieee80211Mac::scheduleTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "scheduling timeout period\n";
    if (isBroadcast(frameToSend))
        scheduleAt(simTime() + frameDuration(frameToSend->length()), endTimeout);
    else
        // TODO: why don't we add SIFS here?
        scheduleAt(simTime() + frameDuration(frameToSend->length()) + frameDuration(LENGTH_ACK) + SIFSPeriod() + PROCESSING_DELAY, endTimeout);
}

void Ieee80211Mac::cancelTimeoutPeriod()
{
    EV << "cancelling Timeout period\n";
    cancelEvent(endTimeout);
}

void Ieee80211Mac::scheduleRTSTimeoutPeriod()
{
    scheduleAt(simTime() + frameDuration(LENGTH_RTS) + frameDuration(LENGTH_CTS) + PROCESSING_DELAY, endTimeout);
}

void Ieee80211Mac::scheduleReservePeriod(Ieee80211Frame *frame)
{
    simtime_t reserve = frame->getDuration();

    if (endReserve->isScheduled()) {
        reserve = max(reserve, endReserve->arrivalTime() - simTime());
        cancelEvent(endReserve);
    }
    else if (radioState == RadioState::IDLE)
    {
        // NAV: the channel is virtually busy according to the spec
        scheduleAt(simTime(), mediumStateChange);
    }

    EV << "scheduling reserve period for: " << reserve << endl;

    nav = true;
    scheduleAt(simTime() + reserve, endReserve);
}

void Ieee80211Mac::generateBackoffPeriod()
{
    backoffPeriod = BackoffPeriod(currentTransmission(), retryCounter);
    EV << "backoff period set to " << backoffPeriod << endl;
}

void Ieee80211Mac::decreaseBackoffPeriod()
{
    // see spec 9.2.5.2
    simtime_t elapsedBackoffTime = simTime() - endBackoff->sendingTime();
    backoffPeriod -= ((int)(elapsedBackoffTime / SlotPeriod())) * SlotPeriod();
    EV << "backoff period decreased to " << backoffPeriod << endl;
}

void Ieee80211Mac::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void Ieee80211Mac::cancelBackoffPeriod()
{
    EV << "cancelling Backoff period\n";
    cancelEvent(endBackoff);
}
/****************************************************************
 * Frame sender functions.
 */
void Ieee80211Mac::sendACKFrameOnEndSIFS()
{
    Ieee80211Frame *frameToACK = (Ieee80211Frame *)endSIFS->contextPointer();
    endSIFS->setContextPointer(NULL);
    sendACKFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(frameToACK));
    delete frameToACK;
}

void Ieee80211Mac::sendACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    EV << "sending ACK frame\n";
    sendDown(buildACKFrame(frameToACK));
}

void Ieee80211Mac::sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "sending Data frame\n";
    sendDown(buildDataFrame(frameToSend));
}

void Ieee80211Mac::sendBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "sending Broadcast frame\n";
    sendDown(buildBroadcastFrame(frameToSend));
}

void Ieee80211Mac::sendRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "sending RTS frame\n";
    sendDown(buildRTSFrame(frameToSend));
}

void Ieee80211Mac::sendCTSFrameOnEndSIFS()
{
    Ieee80211Frame *rtsFrame = (Ieee80211Frame *)endSIFS->contextPointer();
    endSIFS->setContextPointer(NULL);
    sendCTSFrame(check_and_cast<Ieee80211RTSFrame*>(rtsFrame));
    delete rtsFrame;
}

void Ieee80211Mac::sendCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    EV << "sending CTS frame\n";
    sendDown(buildCTSFrame(rtsFrame));
}

/****************************************************************
 * Frame builder functions.
 */
Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    if (isBroadcast(frameToSend))
        frame->setDuration(0);
    else if (!frameToSend->getMoreFragments())
        frame->setDuration(SIFS + frameDuration(LENGTH_ACK));
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * SIFS + 2 * frameDuration(LENGTH_ACK) + frameDuration(frameToSend->length()));

    return frame;
}

Ieee80211ACKFrame *Ieee80211Mac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("wlan-ack");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - SIFS - frameDuration(LENGTH_ACK));

    return frame;
}

Ieee80211RTSFrame *Ieee80211Mac::buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211RTSFrame *frame = new Ieee80211RTSFrame("wlan-rts");
    frame->setTransmitterAddress(address);
    frame->setReceiverAddress(frameToSend->getReceiverAddress());
    frame->setDuration(3 * SIFS + frameDuration(LENGTH_CTS) +
                       frameDuration(frameToSend->length()) +
                       frameDuration(LENGTH_ACK));

    return frame;
}

Ieee80211CTSFrame *Ieee80211Mac::buildCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("wlan-cts");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - SIFS - frameDuration(LENGTH_CTS));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setDuration(0);
    return frame;
}

/****************************************************************
 * Helper functions.
 */
void Ieee80211Mac::setMode(Mode mode)
{
    if (mode == PCF)
        error("PCF mode not yet supported");

    this->mode = mode;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::currentTransmission()
{
    return (Ieee80211DataOrMgmtFrame *)transmissionQueue.front();
}

void Ieee80211Mac::resetStateVariables()
{
    backoff = false;
    backoffPeriod = 0;
    retryCounter = 1;
    
    if (!transmissionQueue.empty())
        currentTransmission()->setRetry(false);
}

bool Ieee80211Mac::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange || (msg == endReserve && radioState == RadioState::IDLE);
}

bool Ieee80211Mac::isMediumFree()
{
    return radioState == RadioState::IDLE && !endReserve->isScheduled();
}

bool Ieee80211Mac::isBroadcast(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool Ieee80211Mac::isForUs(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress() == address;
}

void Ieee80211Mac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    Ieee80211Frame *temp = transmissionQueue.front();
    transmissionQueue.pop_front();
    delete temp;

    if (queueModule)
    {
        // tell queue module that we've become idle
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }
}

double Ieee80211Mac::frameDuration(int bits)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

void Ieee80211Mac::logState()
{
    EV  << "state information: mode = " << modeName(mode) << ", state = " << fsm.stateName()
        << ", backoff = " << backoff << ", backoffPeriod = " << backoffPeriod
        << ", retryCounter = " << retryCounter << ", radioState = " << radioState
        << ", nav = " << nav << endl;
}

const char *Ieee80211Mac::modeName(int mode)
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
