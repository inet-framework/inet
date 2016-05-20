//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/linklayer/newcsma/NewCsmaMac.h"

namespace inet {

namespace newcsma {

Define_Module(NewCsmaMac);

NewCsmaMac::NewCsmaMac()
{
    endSIFS = nullptr;
    endDIFS = nullptr;
    endBackoff = nullptr;
    endTimeout = nullptr;
    mediumStateChange = nullptr;
    pendingRadioConfigMsg = nullptr;
}

NewCsmaMac::~NewCsmaMac()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endTimeout);
    cancelAndDelete(mediumStateChange);

    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;
}

/****************************************************************
 * Initialization functions.
 */
void NewCsmaMac::initialize(int stage)
{
    MACProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        EV << "Initializing stage 0\n";

        maxQueueSize = par("maxQueueSize");
        bitrate = par("bitrate");
        cwMin = par("cwMin");
        cwMax = par("cwMax");
        retryLimit = par("retryLimit");

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
        dynamic_cast<cModule *>(radio)->subscribe(IRadio::receptionStateChangedSignal, this);

        // initalize self messages
        endSIFS = new cMessage("SIFS");
        endDIFS = new cMessage("DIFS");
        endBackoff = new cMessage("Backoff");
        endTimeout = new cMessage("Timeout");
        mediumStateChange = new cMessage("MediumStateChange");

        // obtain pointer to external queue
        initializeQueueModule();

        // state variables
        fsm.setName("NewCsmaMac State Machine");
        retryCounter = 0;
        backoffPeriod = -1;
        backoff = false;

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numCollision = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;

        // initialize watches
        WATCH(fsm);
        WATCH(retryCounter);
        WATCH(backoff);

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

InterfaceEntry *NewCsmaMac::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu"));
    e->setMulticast(true);
    e->setBroadcast(true);
    e->setPointToPoint(false);

    return e;
}

void NewCsmaMac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
        // needed for backoff: mandatory if next message is already present
        queueModule->requestPacket();
    }
}

/****************************************************************
 * Message handling functions.
 */
void NewCsmaMac::handleSelfMsg(cMessage *msg)
{
    EV << "received self message: " << msg << endl;

    handleWithFSM(msg);
}

void NewCsmaMac::handleUpperPacket(cPacket *msg)
{
    // check for queue overflow
    if (maxQueueSize && (int)transmissionQueue.size() == maxQueueSize)
    {
        EV << "message " << msg << " received from higher layer but MAC queue is full, dropping message\n";
        delete msg;
        return;
    }
    NewCsmaDataFrame *frame = check_and_cast<NewCsmaDataFrame *>(msg);
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);

    transmissionQueue.push_back(frame);

    handleWithFSM(frame);
}

void NewCsmaMac::handleLowerPacket(cPacket *msg)
{
    EV << "received message from lower layer: " << msg << endl;

    NewCsmaFrame *frame = dynamic_cast<NewCsmaFrame *>(msg);
    if (!frame)
        error("message from physical layer (%s)%s is not a subclass of NewCsmaFrame",
              msg->getClassName(), msg->getName());

    EV << "Self address: " << address
       << ", receiver address: " << frame->getReceiverAddress()
       << ", received frame is for us: " << isForUs(frame) << endl;

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
}

void NewCsmaMac::receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal) {
        handleWithFSM(mediumStateChange);
    }
}

/**
 * Msg can be upper, lower, self or nullptr (when radio state changes)
 */
void NewCsmaMac::handleWithFSM(cMessage *msg)
{
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMessage(msg) && fsm.getState() != IDLE)
    {
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return;
    }

    NewCsmaFrame *frame = dynamic_cast<NewCsmaFrame*>(msg);
    int frameType = frame ? frame->getType() : -1;

    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Data-Ready,
                                  isUpperMessage(msg),
                                  DEFER,
                ASSERT(isInvalidBackoffPeriod() || backoffPeriod == 0);
                invalidateBackoffPeriod();
            );
            FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     !transmissionQueue.empty(),
                                     DEFER,
                invalidateBackoffPeriod();
            );
            FSMA_Event_Transition(Receive,
                                  isLowerMessage(msg),
                                  RECEIVE,
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Wait-DIFS,
                                  isMediumStateChange(msg) && isMediumFree(),
                                  WAITDIFS,
            ;);
            FSMA_No_Event_Transition(Immediate-Wait-DIFS,
                                     isMediumFree() || !backoff,
                                     WAITDIFS,
            ;);
            FSMA_Event_Transition(Receive,
                                  isLowerMessage(msg),
                                  RECEIVE,
            ;);
        }
        FSMA_State(WAITDIFS)
        {
            FSMA_Enter(scheduleDIFSPeriod());
            FSMA_Event_Transition(Immediate-Transmit-Broadcast,
                                  msg == endDIFS && isBroadcast(getCurrentTransmission()) && !backoff,
                                  WAITBROADCAST,
                sendBroadcastFrame(getCurrentTransmission());
                cancelDIFSPeriod();
            );
            FSMA_Event_Transition(Immediate-Transmit-Data,
                                  msg == endDIFS && !isBroadcast(getCurrentTransmission()) && !backoff,
                                  WAITACK,
                sendDataFrame(getCurrentTransmission());
                cancelDIFSPeriod();
            );
            FSMA_Event_Transition(DIFS-Over,
                                  msg == endDIFS,
                                  BACKOFF,
                ASSERT(backoff);
                if (isInvalidBackoffPeriod())
                    generateBackoffPeriod();
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
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                                  isLowerMessage(msg),
                                  RECEIVE,
                cancelDIFSPeriod();
            ;);
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffPeriod());
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endBackoff && isBroadcast(getCurrentTransmission()),
                                  WAITBROADCAST,
                sendBroadcastFrame(getCurrentTransmission());
            );
            FSMA_Event_Transition(Transmit-Data,
                                  msg == endBackoff && !isBroadcast(getCurrentTransmission()),
                                  WAITACK,
                sendDataFrame(getCurrentTransmission());
            );
            FSMA_Event_Transition(Backoff-Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                cancelBackoffPeriod();
                decreaseBackoffPeriod();
            );
        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter(scheduleDataTimeoutPeriod(getCurrentTransmission()));
            FSMA_Event_Transition(Receive-ACK,
                                  isLowerMessage(msg) && isForUs(frame) && frameType == ST_ACK,
                                  IDLE,
                if (retryCounter == 0) numSentWithoutRetry++;
                numSent++;
                cancelTimeoutPeriod();
                finishCurrentTransmission();
            );
            FSMA_Event_Transition(Transmit-Data-Failed,
                                  msg == endTimeout && retryCounter == retryLimit - 1,
                                  IDLE,
                giveUpCurrentTransmission();
            );
            FSMA_Event_Transition(Receive-ACK-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                retryCurrentTransmission();
            );
        }
        FSMA_State(WAITBROADCAST)
        {
            FSMA_Enter(scheduleBroadcastTimeoutPeriod(getCurrentTransmission()));
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endTimeout,
                                  IDLE,
                finishCurrentTransmission();
                numSentBroadcast++;
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-ACK,
                                  msg == endSIFS,
                                  IDLE,
                sendACKFrame();
                resetStateVariables();
            );
        }
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                                     isLowerMessage(msg) && frame->hasBitError(),
                                     IDLE,
                numCollision++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Broadcast,
                                     isLowerMessage(msg) && isBroadcast(frame),
                                     IDLE,
                sendUp(frame);
                numReceivedBroadcast++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                                     isLowerMessage(msg) && isForUs(frame),
                                     WAITSIFS,
                sendUp(frame);
                numReceived++;
            );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                                     isLowerMessage(msg),
                                     IDLE,
                resetStateVariables();
            );
        }
    }
}

/****************************************************************
 * Timing functions.
 */
simtime_t NewCsmaMac::getSIFS()
{
    return 10E-6;
}

simtime_t NewCsmaMac::getSlotTime()
{
    return 20E-6;
}

simtime_t NewCsmaMac::getDIFS()
{
    return getSIFS() + 2 * getSlotTime();
}

simtime_t NewCsmaMac::computeBackoffPeriod(NewCsmaFrame *msg, int r)
{
    int cw;

    EV << "generating backoff slot number for retry: " << r << endl;

    ASSERT(0 <= r && r < retryLimit);

    cw = (cwMin + 1) * (1 << r) - 1;

    if (cw > cwMax)
        cw = cwMax;

    int c = intrand(cw + 1);

    EV << "generated backoff slot number: " << c << " , cw: " << cw << endl;

    return ((double)c) * getSlotTime();
}

/****************************************************************
 * Timer functions.
 */
void NewCsmaMac::scheduleSIFSPeriod(NewCsmaFrame *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSIFS(), endSIFS);
}

void NewCsmaMac::scheduleDIFSPeriod()
{
    EV << "scheduling DIFS period\n";
    scheduleAt(simTime() + getDIFS(), endDIFS);
}

void NewCsmaMac::cancelDIFSPeriod()
{
    EV << "cancelling DIFS period\n";
    cancelEvent(endDIFS);
}

void NewCsmaMac::scheduleDataTimeoutPeriod(NewCsmaDataFrame *frameToSend)
{
    EV << "scheduling data timeout period\n";
    scheduleAt(simTime() + computeFrameDuration(frameToSend) + getSIFS() + computeFrameDuration(LENGTH_ACK, bitrate) + MAX_PROPAGATION_DELAY * 2, endTimeout);
}

void NewCsmaMac::scheduleBroadcastTimeoutPeriod(NewCsmaDataFrame *frameToSend)
{
    EV << "scheduling broadcast timeout period\n";
    scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
}

void NewCsmaMac::cancelTimeoutPeriod()
{
    EV << "cancelling timeout period\n";
    cancelEvent(endTimeout);
}

void NewCsmaMac::invalidateBackoffPeriod()
{
    backoffPeriod = -1;
}

bool NewCsmaMac::isInvalidBackoffPeriod()
{
    return backoffPeriod == -1;
}

void NewCsmaMac::generateBackoffPeriod()
{
    backoffPeriod = computeBackoffPeriod(getCurrentTransmission(), retryCounter);
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period set to " << backoffPeriod << endl;
}

void NewCsmaMac::decreaseBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffPeriod -= ((int)(elapsedBackoffTime / getSlotTime())) * getSlotTime();
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period decreased to " << backoffPeriod << endl;
}

void NewCsmaMac::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void NewCsmaMac::cancelBackoffPeriod()
{
    EV << "cancelling Backoff period\n";
    cancelEvent(endBackoff);
}

/****************************************************************
 * Frame sender functions.
 */
void NewCsmaMac::sendACKFrame()
{
    NewCsmaFrame *frameToACK = (NewCsmaFrame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(nullptr);
    sendACKFrame(check_and_cast<NewCsmaDataFrame*>(frameToACK));
    delete frameToACK;
}

void NewCsmaMac::sendACKFrame(NewCsmaDataFrame *frameToACK)
{
    EV << "sending ACK frame\n";
    sendDown(buildACKFrame(frameToACK));
}

void NewCsmaMac::sendDataFrame(NewCsmaDataFrame *frameToSend)
{
    EV << "sending Data frame\n";
    sendDown(buildDataFrame(frameToSend));
}

void NewCsmaMac::sendBroadcastFrame(NewCsmaDataFrame *frameToSend)
{
    EV << "sending Broadcast frame\n";
    sendDown(buildBroadcastFrame(frameToSend));
}

/****************************************************************
 * Frame builder functions.
 */
NewCsmaDataFrame *NewCsmaMac::buildDataFrame(NewCsmaDataFrame *frameToSend)
{
    NewCsmaDataFrame *frame = (NewCsmaDataFrame *)frameToSend->dup();
    return frame;
}

NewCsmaAckFrame *NewCsmaMac::buildACKFrame(NewCsmaDataFrame *frameToACK)
{
    NewCsmaAckFrame *frame = new NewCsmaAckFrame("wlan-ack");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());
    return frame;
}

NewCsmaDataFrame *NewCsmaMac::buildBroadcastFrame(NewCsmaDataFrame *frameToSend)
{
    NewCsmaDataFrame *frame = (NewCsmaDataFrame *)frameToSend->dup();
    return frame;
}

/****************************************************************
 * Helper functions.
 */
void NewCsmaMac::finishCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
}

void NewCsmaMac::giveUpCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
    numGivenUp++;
}

void NewCsmaMac::retryCurrentTransmission()
{
    ASSERT(retryCounter < retryLimit - 1);
    getCurrentTransmission()->setRetry(true);
    retryCounter++;
    numRetry++;
    backoff = true;
    generateBackoffPeriod();
}

NewCsmaDataFrame *NewCsmaMac::getCurrentTransmission()
{
    return (NewCsmaDataFrame *)transmissionQueue.front();
}

void NewCsmaMac::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != nullptr)
    {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = nullptr;
    }
}

void NewCsmaMac::resetStateVariables()
{
    backoffPeriod = 0;
    retryCounter = 0;

    if (!transmissionQueue.empty()) {
        backoff = true;
        getCurrentTransmission()->setRetry(false);
    }
    else {
        backoff = false;
    }
}

bool NewCsmaMac::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange;
}

bool NewCsmaMac::isMediumFree()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
}

bool NewCsmaMac::isBroadcast(NewCsmaFrame *frame)
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool NewCsmaMac::isForUs(NewCsmaFrame *frame)
{
    return frame && frame->getReceiverAddress() == address;
}

void NewCsmaMac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    NewCsmaFrame *temp = transmissionQueue.front();
    transmissionQueue.pop_front();
    delete temp;

    if (queueModule)
    {
        // tell queue module that we've become idle
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }
}

double NewCsmaMac::computeFrameDuration(NewCsmaFrame *msg)
{
    return computeFrameDuration(msg->getBitLength(), bitrate);
}

double NewCsmaMac::computeFrameDuration(int bits, double bitrate)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

} // namespace newcsma

} // namespace inet
