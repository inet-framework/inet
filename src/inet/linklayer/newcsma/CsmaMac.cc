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
    cancelAndDelete(endAck);
    cancelAndDelete(endData);
    cancelAndDelete(mediumStateChange);
    for (auto frame : transmissionQueue)
        delete frame;;
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
        useAck = par("useAck");
        bitrate = par("bitrate");
        headerLength = par("headerLength");
        slotTime = par("slotTime");
        sifsTime = par("sifsTime");
        difsTime = par("difsTime");
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
        endAck = new cMessage("Ack");
        endData = new cMessage("Data");
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

void CsmaMac::finish()
{
    recordScalar("numRetry", numRetry);
    recordScalar("numSentWithoutRetry", numSentWithoutRetry);
    recordScalar("numGivenUp", numGivenUp);
    recordScalar("numCollision", numCollision);
    recordScalar("numSent", numSent);
    recordScalar("numReceived", numReceived);
    recordScalar("numSentBroadcast", numSentBroadcast);
    recordScalar("numReceivedBroadcast", numReceivedBroadcast);
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
    CsmaMacDataFrame *frame = encapsulate(msg);
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;
    ASSERT(!frame->getReceiverAddress().isUnspecified());
    transmissionQueue.push_back(frame);
    // skip those cases where there's nothing to do, so the switch looks simpler
    if (fsm.getState() != IDLE)
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
    else
        handleWithFsm(frame);
}

void CsmaMac::handleLowerPacket(cPacket *msg)
{
    EV << "received message from lower layer: " << msg << endl;

    CsmaMacFrame *frame = check_and_cast<CsmaMacFrame *>(msg);
    EV << "Self address: " << address
       << ", receiver address: " << frame->getReceiverAddress()
       << ", received frame is for us: " << isForUs(frame) << endl;

    handleWithFsm(msg);
}

/**
 * Msg can be upper, lower, self or nullptr (when radio state changes)
 */
void CsmaMac::handleWithFsm(cMessage *msg)
{
    CsmaMacFrame *frame = dynamic_cast<CsmaMacFrame*>(msg);
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
            FSMA_Event_Transition(Wait-Difs,
                                  msg == mediumStateChange && isMediumFree(),
                                  WAITDIFS,
            ;);
            FSMA_No_Event_Transition(Immediate-Wait-Difs,
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
            FSMA_Event_Transition(Difs-Over-Transmit,
                                  msg == endDifs && !backoff,
                                  WAITTRANSMIT,
                cancelDifsPeriod();
            );
            FSMA_Event_Transition(Difs-Over-Backoff,
                                  msg == endDifs && backoff,
                                  BACKOFF,
                if (isInvalidBackoffPeriod())
                    generateBackoffPeriod();
            );
            FSMA_Event_Transition(Busy,
                                  msg == mediumStateChange && !isMediumFree(),
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
            FSMA_Event_Transition(Backoff-Done,
                                  msg == endBackoff,
                                  WAITTRANSMIT,
            );
            FSMA_Event_Transition(Backoff-Busy,
                                  msg == mediumStateChange && !isMediumFree(),
                                  DEFER,
                cancelBackoffPeriod();
                decreaseBackoffPeriod();
            );
        }
        FSMA_State(WAITTRANSMIT)
        {
            FSMA_Enter(sendDataFrame(getCurrentTransmission()));
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endData && isBroadcast(getCurrentTransmission()),
                                  IDLE,
                finishCurrentTransmission();
                numSentBroadcast++;
            );
            FSMA_Event_Transition(Transmit-Unicast-Without-Ack,
                                  msg == endData && !useAck && !isBroadcast(getCurrentTransmission()),
                                  IDLE,
                finishCurrentTransmission();
                numSent++;
            );
            FSMA_Event_Transition(Transmit-Unicast-With-Ack,
                                  msg == endData && useAck && !isBroadcast(getCurrentTransmission()),
                                  WAITACK,
            );
        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter(scheduleAckTimeoutPeriod(getCurrentTransmission()));
            FSMA_Event_Transition(Receive-Ack,
                                  isLowerMessage(msg) && isForUs(frame) && dynamic_cast<CsmaMacAckFrame *>(frame),
                                  IDLE,
                if (retryCounter == 0) numSentWithoutRetry++;
                numSent++;
                cancelAckTimeoutPeriod();
                finishCurrentTransmission();
                delete frame;
            );
            FSMA_Event_Transition(Transmit-Failed,
                                  msg == endAck && retryCounter == retryLimit - 1,
                                  IDLE,
                giveUpCurrentTransmission();
            );
            FSMA_Event_Transition(Receive-Ack-Timeout,
                                  msg == endAck,
                                  DEFER,
                retryCurrentTransmission();
            );
        }
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                                     isLowerMessage(msg) && frame->hasBitError(),
                                     IDLE,
                delete frame;
                numCollision++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Broadcast,
                                     isLowerMessage(msg) && isBroadcast(frame),
                                     IDLE,
                sendUp(decapsulate(check_and_cast<CsmaMacDataFrame *>(frame)));
                numReceivedBroadcast++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Without-Ack,
                                     isLowerMessage(msg) && isForUs(frame) && !useAck,
                                     IDLE,
                sendUp(decapsulate(check_and_cast<CsmaMacDataFrame *>(frame)));
                numReceived++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-With-Ack,
                                     isLowerMessage(msg) && isForUs(frame) && useAck,
                                     WAITSIFS,
                sendUp(decapsulate(check_and_cast<CsmaMacDataFrame *>(frame->dup())));
                numReceived++;
            );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                                     isLowerMessage(msg),
                                     IDLE,
                delete frame;
                resetStateVariables();
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSifsPeriod(frame));
            FSMA_Event_Transition(Transmit-Ack,
                                  msg == endSifs,
                                  IDLE,
                sendAckFrame();
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
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            handleWithFsm(endData);
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        }
        transmissionState = newRadioTransmissionState;
    }
}

CsmaMacDataFrame *CsmaMac::encapsulate(cPacket *msg)
{
    CsmaMacDataFrame *frame = new CsmaMacDataFrame(msg->getName());
    frame->setByteLength(headerLength);
    // TODO: kludge to make isUpperMessage work
    frame->setArrival(msg->getArrivalModuleId(), msg->getArrivalGateId());

    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setTransmitterAddress(address);
    frame->setReceiverAddress(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

cPacket *CsmaMac::decapsulate(CsmaMacDataFrame *frame)
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
 * Timer functions.
 */
void CsmaMac::scheduleSifsPeriod(CsmaMacFrame *frame)
{
    EV << "scheduling SIFS period\n";
    endSifs->setContextPointer(frame);
    scheduleAt(simTime() + sifsTime, endSifs);
}

void CsmaMac::scheduleDifsPeriod()
{
    EV << "scheduling DIFS period\n";
    scheduleAt(simTime() + difsTime, endDifs);
}

void CsmaMac::cancelDifsPeriod()
{
    EV << "canceling DIFS period\n";
    cancelEvent(endDifs);
}

void CsmaMac::scheduleAckTimeoutPeriod(CsmaMacDataFrame *frameToSend)
{
    EV << "scheduling ack timeout period\n";
    simtime_t maxPropagationDelay = 2E-6;  // 300 meters at the speed of light
    // TODO: how do we get this?
    int phyHeaderLength = 192;
    double phyHeaderBitrate = 1E+6;
    simtime_t ackDuration = headerLength * 8 / bitrate + phyHeaderLength / phyHeaderBitrate;
    scheduleAt(simTime() + sifsTime + ackDuration + maxPropagationDelay * 2, endAck);
}

void CsmaMac::cancelAckTimeoutPeriod()
{
    EV << "canceling ack timeout period\n";
    cancelEvent(endAck);
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
    return ((double)c) * slotTime;
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
    backoffPeriod -= ((int)(elapsedBackoffTime / slotTime)) * slotTime;
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
void CsmaMac::sendDataFrame(CsmaMacDataFrame *frameToSend)
{
    EV << "sending Data frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(frameToSend->dup());
}
void CsmaMac::sendAckFrame()
{
    EV << "sending Ack frame\n";
    auto frameToAck = static_cast<CsmaMacDataFrame *>(endSifs->getContextPointer());
    endSifs->setContextPointer(nullptr);
    auto ackFrame = new CsmaMacAckFrame("CsmaMacAck");
    ackFrame->setReceiverAddress(frameToAck->getTransmitterAddress());
    ackFrame->setByteLength(headerLength);
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(ackFrame);
    delete frameToAck;
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

CsmaMacDataFrame *CsmaMac::getCurrentTransmission()
{
    return transmissionQueue.front();
}

void CsmaMac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    CsmaMacFrame *temp = transmissionQueue.front();
    transmissionQueue.pop_front();
    delete temp;
    if (queueModule) {
        // tell queue module that we've become idle
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }
}

void CsmaMac::resetStateVariables()
{
    backoffPeriod = 0;
    retryCounter = 0;
    backoff = !transmissionQueue.empty();
}

bool CsmaMac::isMediumFree()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
}

bool CsmaMac::isBroadcast(CsmaMacFrame *frame)
{
    return frame->getReceiverAddress().isBroadcast();
}

bool CsmaMac::isForUs(CsmaMacFrame *frame)
{
    return frame->getReceiverAddress() == address;
}

} // namespace inet
