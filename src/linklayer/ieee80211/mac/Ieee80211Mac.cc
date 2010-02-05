//
// Copyright (C) 2006 Andras Varga and Levente M�sz�ros
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

#include <algorithm>
#include "Ieee80211Mac.h"
#include "RadioState.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "PhyControlInfo_m.h"

Define_Module(Ieee80211Mac);

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(Ieee80211Mac,
   (Ieee80211Mac::IDLE,
    Ieee80211Mac::DEFER,
    Ieee80211Mac::WAITDIFS,
    Ieee80211Mac::BACKOFF,
    Ieee80211Mac::WAITACK,
    Ieee80211Mac::WAITBROADCAST,
    Ieee80211Mac::WAITCTS,
    Ieee80211Mac::WAITSIFS,
    Ieee80211Mac::RECEIVE));

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(RadioState,
   (RadioState::IDLE,
    RadioState::RECV,
    RadioState::TRANSMIT,
    RadioState::SLEEP));

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
    pendingRadioConfigMsg = NULL;
}

Ieee80211Mac::~Ieee80211Mac()
{
    cancelAndDelete(endSIFS);
    cancelAndDelete(endDIFS);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(mediumStateChange);

    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;
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
        basicBitrate = 2e6; //FIXME make it parameter
        rtsThreshold = par("rtsThresholdBytes");

        // the variable is renamed due to a confusion in the standard
        // the name retry limit would be misleading, see the header file comment
        transmissionLimit = par("retryLimit");
        if (transmissionLimit == -1) transmissionLimit = 7;
        ASSERT(transmissionLimit > 0);

        cwMinData = par("cwMinData");
        if (cwMinData == -1) cwMinData = CW_MIN;
        ASSERT(cwMinData >= 0);

        cwMinBroadcast = par("cwMinBroadcast");
        if (cwMinBroadcast == -1) cwMinBroadcast = 31;
        ASSERT(cwMinBroadcast >= 0);

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
        retryCounter = 0;
        backoffPeriod = -1;
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
        stateVector.setEnum("Ieee80211Mac");
        radioStateVector.setName("RadioState");
        radioStateVector.setEnum("RadioState");

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
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

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

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // FIXME: MTU on 802.11 = ?
    e->setMtu(par("mtu"));

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
void Ieee80211Mac::handleSelfMsg(cMessage *msg)
{
    EV << "received self message: " << msg << endl;

    if (msg == endReserve)
        nav = false;

    handleWithFSM(msg);
}

void Ieee80211Mac::handleUpperMsg(cPacket *msg)
{
    // check for queue overflow
    if (maxQueueSize && (int)transmissionQueue.size() == maxQueueSize)
    {
        EV << "message " << msg << " received from higher layer but MAC queue is full, dropping message\n";
        delete msg;
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->getByteLength() > fragmentationThreshold)
        error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->getClassName(), msg->getName(), (int)(msg->getByteLength()));
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!

    transmissionQueue.push_back(frame);

    handleWithFSM(frame);
}

void Ieee80211Mac::handleCommand(cMessage *msg)
{
    if (msg->getKind()==PHY_C_CONFIGURERADIO)
    {
        EV << "Passing on command " << msg->getName() << " to physical layer\n";
        if (pendingRadioConfigMsg != NULL)
        {
            // merge contents of the old command into the new one, then delete it
            PhyControlInfo *pOld = check_and_cast<PhyControlInfo *>(pendingRadioConfigMsg->getControlInfo());
            PhyControlInfo *pNew = check_and_cast<PhyControlInfo *>(msg->getControlInfo());
            if (pNew->getChannelNumber()==-1 && pOld->getChannelNumber()!=-1)
                pNew->setChannelNumber(pOld->getChannelNumber());
            if (pNew->getBitrate()==-1 && pOld->getBitrate()!=-1)
                pNew->setBitrate(pOld->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = NULL;
        }

        if (fsm.getState() == IDLE || fsm.getState() == DEFER || fsm.getState() == BACKOFF)
        {
            EV << "Sending it down immediately\n";
            sendDown(msg);
        }
        else
        {
            EV << "Delaying " << msg->getName() << " until next IDLE or DEFER state\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else
    {
        error("Unrecognized command from mgmt layer: (%s)%s msgkind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

void Ieee80211Mac::handleLowerMsg(cPacket *msg)
{
    EV << "received message from lower layer: " << msg << endl;

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame *>(msg);
    if (!frame)
        error("message from physical layer (%s)%s is not a subclass of Ieee80211Frame",
              msg->getClassName(), msg->getName());

    EV << "Self address: " << address
       << ", receiver address: " << frame->getReceiverAddress()
       << ", received frame is for us: " << isForUs(frame) << endl;

    Ieee80211TwoAddressFrame *twoAddressFrame = dynamic_cast<Ieee80211TwoAddressFrame *>(msg);
    ASSERT(!twoAddressFrame || twoAddressFrame->getTransmitterAddress() != address);

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
}

void Ieee80211Mac::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_RADIOSTATE_CHANGED)
    {
        RadioState::State newRadioState = check_and_cast<RadioState *>(details)->getState();

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
    if (isUpperMsg(msg) && fsm.getState() != IDLE)
    {
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
        return;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame*>(msg);
    int frameType = frame ? frame->getType() : -1;
    int msgKind = msg->getKind();
    logState();
    stateVector.record(fsm.getState());

    if (frame && isLowerMsg(frame))
    {
        lastReceiveFailed =(msgKind == COLLISION || msgKind == BITERROR);
        scheduleReservePeriod(frame);
    }

    // TODO: fix bug according to the message: [omnetpp] A possible bug in the Ieee80211's FSM.
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Data-Ready,
                                  isUpperMsg(msg),
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
                                  isLowerMsg(msg),
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
                                  isLowerMsg(msg),
                                  RECEIVE,
            ;);
        }
        FSMA_State(WAITDIFS)
        {
            FSMA_Enter(scheduleDIFSPeriod());
            FSMA_Event_Transition(Immediate-Transmit-RTS,
                                  msg == endDIFS && !isBroadcast(getCurrentTransmission())
                                  && getCurrentTransmission()->getByteLength() >= rtsThreshold && !backoff,
                                  WAITCTS,
                sendRTSFrame(getCurrentTransmission());
                cancelDIFSPeriod();
            );
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
                                  isLowerMsg(msg),
                                  RECEIVE,
                cancelDIFSPeriod();
            ;);
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffPeriod());
            FSMA_Event_Transition(Transmit-RTS,
                                  msg == endBackoff && !isBroadcast(getCurrentTransmission())
                                  && getCurrentTransmission()->getByteLength() >= rtsThreshold,
                                  WAITCTS,
                sendRTSFrame(getCurrentTransmission());
            );
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
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_ACK,
                                  IDLE,
                if (retryCounter == 0) numSentWithoutRetry++;
                numSent++;
                cancelTimeoutPeriod();
                finishCurrentTransmission();
            );
            FSMA_Event_Transition(Transmit-Data-Failed,
                                  msg == endTimeout && retryCounter == transmissionLimit - 1,
                                  IDLE,
                giveUpCurrentTransmission();
            );
            FSMA_Event_Transition(Receive-ACK-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                retryCurrentTransmission();
            );
        }
        // wait until broadcast is sent
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
        // accoriding to 9.2.5.7 CTS procedure
        FSMA_State(WAITCTS)
        {
            FSMA_Enter(scheduleCTSTimeoutPeriod());
            FSMA_Event_Transition(Receive-CTS,
                                  isLowerMsg(msg) && isForUs(frame) && frameType == ST_CTS,
                                  WAITSIFS,
                cancelTimeoutPeriod();
            );
            FSMA_Event_Transition(Transmit-RTS-Failed,
                                  msg == endTimeout && retryCounter == transmissionLimit - 1,
                                  IDLE,
                giveUpCurrentTransmission();
            );
            FSMA_Event_Transition(Receive-CTS-Timeout,
                                  msg == endTimeout,
                                  DEFER,
                retryCurrentTransmission();
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-CTS,
                                  msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_RTS,
                                  IDLE,
                sendCTSFrameOnEndSIFS();
                resetStateVariables();
            );
            FSMA_Event_Transition(Transmit-DATA,
                                  msg == endSIFS && getFrameReceivedBeforeSIFS()->getType() == ST_CTS,
                                  WAITACK,
                sendDataFrameOnEndSIFS(getCurrentTransmission());
            );
            FSMA_Event_Transition(Transmit-ACK,
                                  msg == endSIFS && isDataOrMgmtFrame(getFrameReceivedBeforeSIFS()),
                                  IDLE,
                sendACKFrameOnEndSIFS();
                resetStateVariables();
            );
        }
        // this is not a real state
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                                     isLowerMsg(msg) && (msgKind == COLLISION || msgKind == BITERROR),
                                     IDLE,
                EV << "received frame contains bit errors or collision, next wait period is EIFS\n";
                numCollision++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Broadcast,
                                     isLowerMsg(msg) && isBroadcast(frame) && isDataOrMgmtFrame(frame),
                                     IDLE,
                sendUp(frame);
                numReceivedBroadcast++;
                resetStateVariables();
            );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                                     isLowerMsg(msg) && isForUs(frame) && isDataOrMgmtFrame(frame),
                                     WAITSIFS,
                sendUp(frame);
                numReceived++;
            );
            FSMA_No_Event_Transition(Immediate-Receive-RTS,
                                     isLowerMsg(msg) && isForUs(frame) && frameType == ST_RTS,
                                     WAITSIFS,
            );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                                     isLowerMsg(msg),
                                     IDLE,
                resetStateVariables();
            );
        }
    }

    logState();
    stateVector.record(fsm.getState());
}

/****************************************************************
 * Timing functions.
 */
simtime_t Ieee80211Mac::getSIFS()
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    return SIFS;
}

simtime_t Ieee80211Mac::getSlotTime()
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    return ST;
}

simtime_t Ieee80211Mac::getPIFS()
{
    return getSIFS() + getSlotTime();
}

simtime_t Ieee80211Mac::getDIFS()
{
    return getSIFS() + 2 * getSlotTime();
}

simtime_t Ieee80211Mac::getEIFS()
{
// FIXME:   return getSIFS() + getDIFS() + (8 * ACKSize + aPreambleLength + aPLCPHeaderLength) / lowestDatarate;
    return getSIFS() + getDIFS() + (8 * LENGTH_ACK + PHY_HEADER_LENGTH) / 1E+6;
}

simtime_t Ieee80211Mac::computeBackoffPeriod(Ieee80211Frame *msg, int r)
{
    int cw;

    EV << "generating backoff slot number for retry: " << r << endl;

    if (isBroadcast(msg))
        cw = cwMinBroadcast;
    else
    {
        ASSERT(0 <= r && r < transmissionLimit);

        cw = (cwMinData + 1) * (1 << r) - 1;

        if (cw > CW_MAX)
            cw = CW_MAX;
    }

    int c = intrand(cw + 1);

    EV << "generated backoff slot number: " << c << " , cw: " << cw << endl;

    return ((double)c) * getSlotTime();
}

/****************************************************************
 * Timer functions.
 */
void Ieee80211Mac::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSIFS(), endSIFS);
}

void Ieee80211Mac::scheduleDIFSPeriod()
{
    if (lastReceiveFailed)
    {
        EV << "receiption of last frame failed, scheduling EIFS period\n";
        scheduleAt(simTime() + getEIFS(), endDIFS);
    }
    else
    {
        EV << "scheduling DIFS period\n";
        scheduleAt(simTime() + getDIFS(), endDIFS);
    }
}

void Ieee80211Mac::cancelDIFSPeriod()
{
    EV << "cancelling DIFS period\n";
    cancelEvent(endDIFS);
}

void Ieee80211Mac::scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "scheduling data timeout period\n";
    scheduleAt(simTime() + computeFrameDuration(frameToSend) + getSIFS() + computeFrameDuration(LENGTH_ACK, basicBitrate) + MAX_PROPAGATION_DELAY * 2, endTimeout);
}

void Ieee80211Mac::scheduleBroadcastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV << "scheduling broadcast timeout period\n";
    scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
}

void Ieee80211Mac::cancelTimeoutPeriod()
{
    EV << "cancelling timeout period\n";
    cancelEvent(endTimeout);
}

void Ieee80211Mac::scheduleCTSTimeoutPeriod()
{
    scheduleAt(simTime() + computeFrameDuration(LENGTH_RTS, basicBitrate) + getSIFS() + computeFrameDuration(LENGTH_CTS, basicBitrate) + MAX_PROPAGATION_DELAY * 2, endTimeout);
}

void Ieee80211Mac::scheduleReservePeriod(Ieee80211Frame *frame)
{
    simtime_t reserve = frame->getDuration();

    // see spec. 7.1.3.2
    if (!isForUs(frame) && reserve != 0 && reserve < 32768)
    {
        if (endReserve->isScheduled()) {
            simtime_t oldReserve = endReserve->getArrivalTime() - simTime();

            if (oldReserve > reserve)
                return;

            reserve = std::max(reserve, oldReserve);
            cancelEvent(endReserve);
        }
        else if (radioState == RadioState::IDLE)
        {
            // NAV: the channel just became virtually busy according to the spec
            scheduleAt(simTime(), mediumStateChange);
        }

        EV << "scheduling reserve period for: " << reserve << endl;

        ASSERT(reserve > 0);

        nav = true;
        scheduleAt(simTime() + reserve, endReserve);
    }
}

void Ieee80211Mac::invalidateBackoffPeriod()
{
    backoffPeriod = -1;
}

bool Ieee80211Mac::isInvalidBackoffPeriod()
{
    return backoffPeriod == -1;
}

void Ieee80211Mac::generateBackoffPeriod()
{
    backoffPeriod = computeBackoffPeriod(getCurrentTransmission(), retryCounter);
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period set to " << backoffPeriod << endl;
}

void Ieee80211Mac::decreaseBackoffPeriod()
{
    // see spec 9.2.5.2
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffPeriod -= ((int)(elapsedBackoffTime / getSlotTime())) * getSlotTime();
    ASSERT(backoffPeriod >= 0);
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
    Ieee80211Frame *frameToACK = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendACKFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(frameToACK));
    delete frameToACK;
}

void Ieee80211Mac::sendACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    EV << "sending ACK frame\n";
    sendDown(setBasicBitrate(buildACKFrame(frameToACK)));
}

void Ieee80211Mac::sendDataFrameOnEndSIFS(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211Frame *ctsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendDataFrame(frameToSend);
    delete ctsFrame;
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
    sendDown(setBasicBitrate(buildRTSFrame(frameToSend)));
}

void Ieee80211Mac::sendCTSFrameOnEndSIFS()
{
    Ieee80211Frame *rtsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(NULL);
    sendCTSFrame(check_and_cast<Ieee80211RTSFrame*>(rtsFrame));
    delete rtsFrame;
}

void Ieee80211Mac::sendCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    EV << "sending CTS frame\n";
    sendDown(setBasicBitrate(buildCTSFrame(rtsFrame)));
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
        frame->setDuration(getSIFS() + computeFrameDuration(LENGTH_ACK, basicBitrate));
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * getSIFS() + 2 * computeFrameDuration(LENGTH_ACK, basicBitrate) + computeFrameDuration(frameToSend));

    return frame;
}

Ieee80211ACKFrame *Ieee80211Mac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("wlan-ack");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - getSIFS() - computeFrameDuration(LENGTH_ACK, basicBitrate));

    return frame;
}

Ieee80211RTSFrame *Ieee80211Mac::buildRTSFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211RTSFrame *frame = new Ieee80211RTSFrame("wlan-rts");
    frame->setTransmitterAddress(address);
    frame->setReceiverAddress(frameToSend->getReceiverAddress());
    frame->setDuration(3 * getSIFS() + computeFrameDuration(LENGTH_CTS, basicBitrate) +
                       computeFrameDuration(frameToSend) +
                       computeFrameDuration(LENGTH_ACK, basicBitrate));

    return frame;
}

Ieee80211CTSFrame *Ieee80211Mac::buildCTSFrame(Ieee80211RTSFrame *rtsFrame)
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("wlan-cts");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSIFS() - computeFrameDuration(LENGTH_CTS, basicBitrate));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setDuration(0);
    return frame;
}

Ieee80211Frame *Ieee80211Mac::setBasicBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo()==NULL);
    PhyControlInfo *ctrl = new PhyControlInfo();
    ctrl->setBitrate(basicBitrate);
    frame->setControlInfo(ctrl);
    return frame;
}

/****************************************************************
 * Helper functions.
 */
void Ieee80211Mac::finishCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
}

void Ieee80211Mac::giveUpCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
    numGivenUp++;
}

void Ieee80211Mac::retryCurrentTransmission()
{
    ASSERT(retryCounter < transmissionLimit - 1);
    getCurrentTransmission()->setRetry(true);
    retryCounter++;
    numRetry++;
    backoff = true;
    generateBackoffPeriod();
}

Ieee80211DataOrMgmtFrame *Ieee80211Mac::getCurrentTransmission()
{
    return (Ieee80211DataOrMgmtFrame *)transmissionQueue.front();
}

void Ieee80211Mac::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != NULL)
    {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = NULL;
    }
}

void Ieee80211Mac::setMode(Mode mode)
{
    if (mode == PCF)
        error("PCF mode not yet supported");

    this->mode = mode;
}

void Ieee80211Mac::resetStateVariables()
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

bool Ieee80211Mac::isDataOrMgmtFrame(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame);
}

Ieee80211Frame *Ieee80211Mac::getFrameReceivedBeforeSIFS()
{
    return (Ieee80211Frame *)endSIFS->getContextPointer();
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

double Ieee80211Mac::computeFrameDuration(Ieee80211Frame *msg)
{
    return computeFrameDuration(msg->getBitLength(), bitrate);
}

double Ieee80211Mac::computeFrameDuration(int bits, double bitrate)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

void Ieee80211Mac::logState()
{
    EV  << "state information: mode = " << modeName(mode) << ", state = " << fsm.getStateName()
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
        CASE(PCF);
    }
    return s;
#undef CASE
}
