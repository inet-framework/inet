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
#include "Ieee802Ctrl_m.h"
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
    radioStateChange = NULL;
}

Ieee80211Mac::~Ieee80211Mac()
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
        radioStateChange = new cMessage("radioStateChange");

        // interface
        registerInterface();

        // obtain pointer to external queue
        initializeQueueModule();

        // state variables
        fsm.setName("Ieee80211Mac State Machine");
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

    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->byteLength() > 2312+34) //XXX use constant
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->byteLength());
    EV << "frame " << frame << " received from higher layer, receiver=" << frame->getReceiverAddress() << endl;
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
    //XXX delete msg; -- this won't work anymore since we send up data frames without decapsulation
}

void Ieee80211Mac::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();

    if (category == NF_RADIOSTATE_CHANGED)
    {
        radioState = check_and_cast<RadioState *>(details)->getState();
        EV << "radio state changed " << className() << ": " << details->info() << endl;

        handleWithFSM(radioStateChange);
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

    if (isRadioStateChange(msg) && fsm.state() != DEFER && fsm.state() != WAITDIFS && fsm.state() != BACKOFF)
    {
        EV << "ignoring radio state change in " << fsm.stateName() << " state\n";
        return;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame*>(msg);
    int frameType = frame ? frame->getType() : -1;
    logState();

    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Event_Transition(Rx-Data,
                                  mode == DCF && isLowerMsg(msg) && frameType == ST_DATA,
                                  WAITSIFS,
                sendUp(frame);
                numReceived++;
            );
            FSMA_Event_Transition(Rx-Broadcast,
                                  mode == DCF && isLowerMsg(msg) && frameType == ST_DATA && isBroadcast(frame),
                                  IDLE,
                sendUp(frame);
                numReceivedBroadcast++;
            );
            FSMA_Event_Transition(Rx-RTS,
                                  mode == MACA && isForUs(frame) && isLowerMsg(msg) && frameType == ST_RTS,
                                  WAITSIFS,
            );
            FSMA_Event_Transition(Reserve,
                                  mode == MACA && !isForUs(frame) && isLowerMsg(msg) && (frameType == ST_DATA || frameType == ST_RTS || frameType == ST_CTS),
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
                                  mode == DCF && isLowerMsg(msg) && frameType == ST_ACK,
                                  IDLE,
                cancelEvent(endTimeout);
                popTransmissionQueue();
                if (retryCounter == 1) numSentWithoutRetry++;
                numSent++;
                resetStateVariables();
            );
            FSMA_Event_Transition(Rx-CTS,
                                  mode == MACA && isLowerMsg(msg) && frameType == ST_CTS,
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
                //XXX extract to function: sendACKFrameOnEndSIFS()
                Ieee80211Frame *frameToAck = (Ieee80211Frame *)endSIFS->contextPointer();
                endSIFS->setContextPointer(NULL);
                sendACKFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(frameToAck));
                delete frameToAck;
            );
            FSMA_Event_Transition(Tx-CTS,
                                  mode == MACA && msg == endSIFS,
                                  IDLE,
                //XXX extract to function: sendCTSFrameOnEndSIFS()
                Ieee80211Frame *frameToAck = (Ieee80211Frame *)endSIFS->contextPointer();
                endSIFS->setContextPointer(NULL);
                sendCTSFrame(check_and_cast<Ieee80211RTSFrame*>(frameToAck));
                delete frameToAck;
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
                                  mode == MACA && !isForUs(frame) && isLowerMsg(msg) && (frameType == ST_DATA || frameType == ST_RTS || frameType == ST_CTS),
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
void Ieee80211Mac::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + SIFS, endSIFS);
}

void Ieee80211Mac::scheduleDIFSPeriod()
{
    EV << "scheduling DIFS period\n";
    scheduleAt(simTime() + DIFS, endDIFS);
}

void Ieee80211Mac::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void Ieee80211Mac::scheduleTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "scheduling timeout period\n";
    if (isBroadcast(frameToSend))
        scheduleAt(simTime() + frameDuration(frameToSend->length()) + PROCESSING_TIMEOUT, endTimeout);
    else
        scheduleAt(simTime() + frameDuration(frameToSend->length()) + frameDuration(LENGTH_ACK) + PROCESSING_TIMEOUT, endTimeout);
}

void Ieee80211Mac::scheduleRTSTimeoutPeriod()
{
    scheduleAt(simTime() + frameDuration(LENGTH_RTS) + frameDuration(LENGTH_CTS) + PROCESSING_TIMEOUT, endTimeout);
}

void Ieee80211Mac::scheduleReservePeriod(Ieee80211Frame *frame)
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
void Ieee80211Mac::sendACKFrame(Ieee80211DataOrMgmtFrame *frameToAck)
{
    EV << "sending ACK frame\n";
    sendDown(buildACKFrame(frameToAck));
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

void Ieee80211Mac::sendCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    EV << "sending CTS frame\n";
    sendDown(buildCTSFrame(rtsFrame));
}


/****************************************************************
 * Frame builder functions.
 */

//XXX rename to dupDataFrame()? BTW, dup() takes care of setting the fields below as well
Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setTransmitterAddress(address);

    if (isBroadcast(frameToSend))
        frame->setDuration(0);
    else //XXX if (frameToSend->getFragmentation() == 0)
        frame->setDuration(SIFS + frameDuration(LENGTH_ACK));
     //XXX else
     //XXX      frame->setDuration(3 * SIFS + 2 * frameDuration(LENGTH_ACK) + frameDuration(frameToSend->length()));

    return frame;
}

Ieee80211ACKFrame *Ieee80211Mac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("wlan-ack");
    frame->setKind(2); // only for debugging: give message a different color in the animation
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    //XXX if (frameToACK->getFragmentation() == 0)
        frame->setDuration(0);
    //XXX else
    //XXX    frame->setDuration(frameToACK->getDuration() - SIFS - frameDuration(LENGTH_ACK));

    return frame;
}

Ieee80211RTSFrame *Ieee80211Mac::buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211RTSFrame *frame = new Ieee80211RTSFrame("wlan-rts");
    frame->setKind(2); // only for debugging: give message a different color in the animation
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
    frame->setKind(2); // only for debugging: give message a different color in the animation
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - SIFS - frameDuration(LENGTH_CTS));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setKind(1); // only for debugging: give message a different color in the animation
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

void Ieee80211Mac::resetStateVariables()
{
    backoff = false;
    backoffPeriod = 0;
    retryCounter = 1;
}

void Ieee80211Mac::generateBackoffPeriod()
{
    backoffPeriod = ((double) intrand(contentionWindow() + 1)) * ST;
    EV << "backoff period set to " << backoffPeriod << endl;
}

int Ieee80211Mac::contentionWindow()
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

bool Ieee80211Mac::isRadioStateChange(cMessage *msg)
{
    return msg == radioStateChange;
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
        << ", retryCounter = " << retryCounter << ", radioState = " << radioState << endl;
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
