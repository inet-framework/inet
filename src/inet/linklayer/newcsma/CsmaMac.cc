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

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/newcsma/CsmaMac.h"

namespace inet {

Define_Module(CsmaMac);

CsmaMac::~CsmaMac()
{
    cancelAndDelete(endSifs);
    cancelAndDelete(endDifs);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endTimeout);
    cancelAndDelete(mediumStateChange);
}

/****************************************************************
 * Initialization functions.
 */
void CsmaMac::initialize(int stage)
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
        registerInterface();

        // subscribe for the information of the carrier sense
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        // initalize self messages
        endSifs = new cMessage("SIFS");
        endDifs = new cMessage("DIFS");
        endBackoff = new cMessage("Backoff");
        endTimeout = new cMessage("Timeout");
        mediumStateChange = new cMessage("MediumStateChange");

        // obtain pointer to external queue
        initializeQueueModule();

        // state variables
        fsm.setName("CsmaMac State Machine");
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
    else if (stage == INITSTAGE_LINK_LAYER)
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
}

InterfaceEntry *CsmaMac::createInterfaceEntry()
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

void CsmaMac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (par("queueModule").stringValue()[0]) {
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
void CsmaMac::handleSelfMessage(cMessage *msg)
{
    EV << "received self message: " << msg << endl;
    handleWithFsm(msg);
}

void CsmaMac::handleUpperPacket(cPacket *msg)
{
    // check for queue overflow
    if (maxQueueSize && (int)transmissionQueue.size() == maxQueueSize) {
        EV << "message " << msg << " received from higher layer but MAC queue is full, dropping message\n";
        delete msg;
        return;
    }
    CsmaDataFrame *frame = encapsulate(msg);
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    transmissionQueue.push_back(frame);
    handleWithFsm(frame);
}

void CsmaMac::handleLowerPacket(cPacket *msg)
{
    EV << "received message from lower layer: " << msg << endl;

    CsmaFrame *frame = check_and_cast<CsmaFrame *>(msg);
    EV << "Self address: " << address
       << ", receiver address: " << frame->getReceiverAddress()
       << ", received frame is for us: " << isForUs(frame) << endl;

    handleWithFsm(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
}

/**
 * Msg can be upper, lower, self or nullptr (when radio state changes)
 */
void CsmaMac::handleWithFsm(cMessage *msg)
{
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (isUpperMessage(msg) && fsm.getState() != IDLE) {
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return;
    }
    CsmaFrame *frame = dynamic_cast<CsmaFrame*>(msg);
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
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
            FSMA_Enter(scheduleDifsPeriod());
            FSMA_Event_Transition(Immediate-Transmit-Broadcast,
                                  msg == endDifs && isBroadcast(getCurrentTransmission()) && !backoff,
                                  WAITBROADCAST,
                sendBroadcastFrame(getCurrentTransmission());
                cancelDifsPeriod();
            );
            FSMA_Event_Transition(Immediate-Transmit-Data,
                                  msg == endDifs && !isBroadcast(getCurrentTransmission()) && !backoff,
                                  WAITACK,
                sendDataFrame(getCurrentTransmission());
                cancelDifsPeriod();
            );
            FSMA_Event_Transition(DIFS-Over,
                                  msg == endDifs,
                                  BACKOFF,
                ASSERT(backoff);
                if (isInvalidBackoffPeriod())
                    generateBackoffPeriod();
            );
            FSMA_Event_Transition(Busy,
                                  isMediumStateChange(msg) && !isMediumFree(),
                                  DEFER,
                backoff = true;
                cancelDifsPeriod();
            );
            FSMA_No_Event_Transition(Immediate-Busy,
                                     !isMediumFree(),
                                     DEFER,
                backoff = true;
                cancelDifsPeriod();
            );
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                                  isLowerMessage(msg),
                                  RECEIVE,
                cancelDifsPeriod();
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
                                  isLowerMessage(msg) && isForUs(frame) && dynamic_cast<CsmaAckFrame *>(frame),
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
            FSMA_Enter(scheduleSifsPeriod(frame));
            FSMA_Event_Transition(Transmit-ACK,
                                  msg == endSifs,
                                  IDLE,
                sendAckFrame();
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
                sendUp(decapsulate(check_and_cast<CsmaDataFrame *>(frame)));
                numReceivedBroadcast++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                                     isLowerMessage(msg) && isForUs(frame),
                                     WAITSIFS,
                sendUp(decapsulate(check_and_cast<CsmaDataFrame *>(frame->dup())));
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

void CsmaMac::receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal)
        handleWithFsm(mediumStateChange);
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        transmissionState = newRadioTransmissionState;
    }
}

CsmaDataFrame *CsmaMac::encapsulate(cPacket *msg)
{
    CsmaDataFrame *frame = new CsmaDataFrame(msg->getName());
    frame->setByteLength(headerLength);
    // TODO: kludge to make isUpperMessage work
    frame->setArrival(msg->getArrivalModuleId(), msg->getArrivalGateId());

    // copy receiver address from the control info (sender address will be set in MAC)
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setReceiverAddress(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

cPacket *CsmaMac::decapsulate(CsmaDataFrame *frame)
{
    cPacket *payload = frame->decapsulate();

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    payload->setControlInfo(ctrl);

    delete frame;
    return payload;
}

/****************************************************************
 * Timing functions.
 */
simtime_t CsmaMac::getSifs()
{
    return 10E-6;
}

simtime_t CsmaMac::getSlotTime()
{
    return 20E-6;
}

simtime_t CsmaMac::getDifs()
{
    return getSifs() + 2 * getSlotTime();
}

simtime_t CsmaMac::computeBackoffPeriod(int r)
{
    ASSERT(0 <= r && r < retryLimit);
    EV << "generating backoff slot number for retry: " << r << endl;
    int cw = (cwMin + 1) * (1 << r) - 1;
    if (cw > cwMax)
        cw = cwMax;
    int c = intrand(cw + 1);
    EV << "generated backoff slot number: " << c << " , cw: " << cw << endl;
    return ((double)c) * getSlotTime();
}

/****************************************************************
 * Timer functions.
 */
void CsmaMac::scheduleSifsPeriod(CsmaFrame *frame)
{
    EV << "scheduling SIFS period\n";
    endSifs->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSifs(), endSifs);
}

void CsmaMac::scheduleDifsPeriod()
{
    EV << "scheduling DIFS period\n";
    scheduleAt(simTime() + getDifs(), endDifs);
}

void CsmaMac::cancelDifsPeriod()
{
    EV << "cancelling DIFS period\n";
    cancelEvent(endDifs);
}

void CsmaMac::scheduleDataTimeoutPeriod(CsmaDataFrame *frameToSend)
{
    EV << "scheduling data timeout period\n";
    simtime_t maxPropagationDelay = 2E-6;  // 300 meters at the speed of light
    scheduleAt(simTime() + computeFrameDuration(frameToSend) + getSifs() + computeFrameDuration(headerLength * 8, bitrate) + maxPropagationDelay * 2, endTimeout);
}

void CsmaMac::scheduleBroadcastTimeoutPeriod(CsmaDataFrame *frameToSend)
{
    EV << "scheduling broadcast timeout period\n";
    scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
}

void CsmaMac::cancelTimeoutPeriod()
{
    EV << "canceling timeout period\n";
    cancelEvent(endTimeout);
}

void CsmaMac::invalidateBackoffPeriod()
{
    backoffPeriod = -1;
}

bool CsmaMac::isInvalidBackoffPeriod()
{
    return backoffPeriod == -1;
}

void CsmaMac::generateBackoffPeriod()
{
    backoffPeriod = computeBackoffPeriod(retryCounter);
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period set to " << backoffPeriod << endl;
}

void CsmaMac::decreaseBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffPeriod -= ((int)(elapsedBackoffTime / getSlotTime())) * getSlotTime();
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period decreased to " << backoffPeriod << endl;
}

void CsmaMac::scheduleBackoffPeriod()
{
    EV << "scheduling backoff period\n";
    scheduleAt(simTime() + backoffPeriod, endBackoff);
}

void CsmaMac::cancelBackoffPeriod()
{
    EV << "canceling backoff period\n";
    cancelEvent(endBackoff);
}

/****************************************************************
 * Frame sender functions.
 */
void CsmaMac::sendAckFrame()
{
    CsmaFrame *frameToAck = (CsmaFrame *)endSifs->getContextPointer();
    endSifs->setContextPointer(nullptr);
    sendAckFrame(check_and_cast<CsmaDataFrame*>(frameToAck));
    delete frameToAck;
}

void CsmaMac::sendAckFrame(CsmaDataFrame *frameToAck)
{
    EV << "sending ACK frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildAckFrame(frameToAck));
}

void CsmaMac::sendDataFrame(CsmaDataFrame *frameToSend)
{
    EV << "sending Data frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildDataFrame(frameToSend));
}

void CsmaMac::sendBroadcastFrame(CsmaDataFrame *frameToSend)
{
    EV << "sending Broadcast frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildBroadcastFrame(frameToSend));
}

/****************************************************************
 * Frame builder functions.
 */
CsmaDataFrame *CsmaMac::buildDataFrame(CsmaDataFrame *frameToSend)
{
    return (CsmaDataFrame *)frameToSend->dup();
}

CsmaAckFrame *CsmaMac::buildAckFrame(CsmaDataFrame *frameToAck)
{
    CsmaAckFrame *frame = new CsmaAckFrame("CsmaAck");
    frame->setReceiverAddress(frameToAck->getTransmitterAddress());
    frame->setByteLength(headerLength);
    return frame;
}

CsmaDataFrame *CsmaMac::buildBroadcastFrame(CsmaDataFrame *frameToSend)
{
    return (CsmaDataFrame *)frameToSend->dup();
}

/****************************************************************
 * Helper functions.
 */
void CsmaMac::finishCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
}

void CsmaMac::giveUpCurrentTransmission()
{
    emit(NF_LINK_BREAK, getCurrentTransmission());
    popTransmissionQueue();
    resetStateVariables();
    numGivenUp++;
}

void CsmaMac::retryCurrentTransmission()
{
    ASSERT(retryCounter < retryLimit - 1);
    backoff = true;
    retryCounter++;
    numRetry++;
    generateBackoffPeriod();
}

CsmaDataFrame *CsmaMac::getCurrentTransmission()
{
    return (CsmaDataFrame *)transmissionQueue.front();
}

void CsmaMac::resetStateVariables()
{
    backoffPeriod = 0;
    retryCounter = 0;
    backoff = !transmissionQueue.empty();
}

bool CsmaMac::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange;
}

bool CsmaMac::isMediumFree()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
}

bool CsmaMac::isBroadcast(CsmaFrame *frame)
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool CsmaMac::isForUs(CsmaFrame *frame)
{
    return frame && frame->getReceiverAddress() == address;
}

void CsmaMac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    CsmaFrame *temp = transmissionQueue.front();
    transmissionQueue.pop_front();
    delete temp;
    if (queueModule) {
        // tell queue module that we've become idle
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }
}

double CsmaMac::computeFrameDuration(CsmaFrame *msg)
{
    return computeFrameDuration(msg->getBitLength(), bitrate);
}

double CsmaMac::computeFrameDuration(int bits, double bitrate)
{
    int phyHeaderLength = 192;
    double phyHeaderBitrate = 1E+6;
    return bits / bitrate + phyHeaderLength / phyHeaderBitrate;
}

} // namespace inet
