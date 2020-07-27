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
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/csmaca/CsmaCaMac.h"

namespace inet {

using namespace inet::physicallayer;

Define_Module(CsmaCaMac);

CsmaCaMac::~CsmaCaMac()
{
    cancelAndDelete(endSifs);
    cancelAndDelete(endDifs);
    cancelAndDelete(endBackoff);
    cancelAndDelete(endAckTimeout);
    cancelAndDelete(endData);
    cancelAndDelete(mediumStateChange);
}

/****************************************************************
 * Initialization functions.
 */
void CsmaCaMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        EV << "Initializing stage 0\n";
        fcsMode = parseFcsMode(par("fcsMode"));
        useAck = par("useAck");
        bitrate = par("bitrate");
        headerLength = B(par("headerLength"));
        if (headerLength > B(255))
            throw cRuntimeError("The specified headerLength is too large");
        if (headerLength < makeShared<CsmaCaMacDataHeader>()->getChunkLength())
            throw cRuntimeError("The specified headerLength is too short");
        ackLength = B(par("ackLength"));
        if (ackLength > B(255))
            throw cRuntimeError("The specified ackLength is too large");
        if (ackLength < makeShared<CsmaCaMacAckHeader>()->getChunkLength())
            throw cRuntimeError("The specified ackLength is too short");
        ackTimeout = par("ackTimeout");
        slotTime = par("slotTime");
        sifsTime = par("sifsTime");
        difsTime = par("difsTime");
        cwMin = par("cwMin");
        cwMax = par("cwMax");
        cwMulticast = par("cwMulticast");
        retryLimit = par("retryLimit");

        // initialize self messages
        endSifs = new cMessage("SIFS");
        endDifs = new cMessage("DIFS");
        endBackoff = new cMessage("Backoff");
        endAckTimeout = new cMessage("AckTimeout");
        endData = new cMessage("Data");
        mediumStateChange = new cMessage("MediumStateChange");

        // set up internal queue
        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));

        // state variables
        fsm.setName("CsmaCaMac State Machine");
        backoffPeriod = -1;
        retryCounter = 0;

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
        WATCH(backoffPeriod);
        WATCH(retryCounter);
        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numCollision);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // subscribe for the information of the carrier sense
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);

        radio = check_and_cast<IRadio *>(radioModule);
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    }
}

void CsmaCaMac::finish()
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

void CsmaCaMac::configureInterfaceEntry()
{
    MacAddress address = parseMacAddressParameter(par("address"));

    // data rate
    interfaceEntry->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMacAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    interfaceEntry->setMtu(par("mtu"));
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);
    interfaceEntry->setPointToPoint(false);
}

/****************************************************************
 * Message handling functions.
 */
void CsmaCaMac::handleSelfMessage(cMessage *msg)
{
    EV << "received self message: " << msg << endl;
    handleWithFsm(msg);
}

void CsmaCaMac::handleUpperPacket(Packet *packet)
{
    auto frame = check_and_cast<Packet *>(packet);
    encapsulate(frame);
    const auto& macHeader = frame->peekAtFront<CsmaCaMacHeader>();
    EV << "frame " << frame << " received from higher layer, receiver = " << macHeader->getReceiverAddress() << endl;
    ASSERT(!macHeader->getReceiverAddress().isUnspecified());
    txQueue->pushPacket(frame);
    if (fsm.getState() != IDLE)
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
    else if (!txQueue->isEmpty()){
        popTxQueue();
        handleWithFsm(currentTxFrame);
    }
}

void CsmaCaMac::handleLowerPacket(Packet *packet)
{
    EV << "received message from lower layer: " << packet << endl;
    handleWithFsm(packet);
}

void CsmaCaMac::handleWithFsm(cMessage *msg)
{
    Packet *frame = dynamic_cast<Packet *>(msg);
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Event_Transition(Defer-Transmit,
                                  isUpperMessage(msg) && !isMediumFree(),
                                  DEFER,
            );
            FSMA_Event_Transition(Start-Backoff,
                                  isUpperMessage(msg) && isMediumFree() && !useAck,
                                  BACKOFF,
            );
            FSMA_Event_Transition(Start-Difs,
                                  isUpperMessage(msg) && isMediumFree() && useAck,
                                  WAITDIFS,
            );
            FSMA_Event_Transition(Start-Receive,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVE,
            );
        }
        FSMA_State(DEFER)
        {
            FSMA_Event_Transition(Start-Backoff,
                                  msg == mediumStateChange && isMediumFree() && !useAck,
                                  BACKOFF,
            );
            FSMA_Event_Transition(Start-Difs,
                                  msg == mediumStateChange && isMediumFree() && useAck,
                                  WAITDIFS,
            );
            FSMA_Event_Transition(Start-Receive,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVE,
            );
        }
        FSMA_State(WAITDIFS)
        {
            FSMA_Enter(scheduleDifsTimer());
            FSMA_Event_Transition(Start-Backoff,
                                  msg == endDifs,
                                  BACKOFF,
            );
            FSMA_Event_Transition(Start-Receive,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVE,
                cancelDifsTimer();
            );
            FSMA_Event_Transition(Defer-Difs,
                                  msg == mediumStateChange && !isMediumFree(),
                                  DEFER,
                cancelDifsTimer();
            );
        }
        FSMA_State(BACKOFF)
        {
            FSMA_Enter(scheduleBackoffTimer());
            FSMA_Event_Transition(Start-Transmit,
                                  msg == endBackoff,
                                  TRANSMIT,
                invalidateBackoffPeriod();
            );
            FSMA_Event_Transition(Start-Receive,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVE,
                cancelBackoffTimer();
                decreaseBackoffPeriod();
            );
            FSMA_Event_Transition(Defer-Backoff,
                                  msg == mediumStateChange && !isMediumFree(),
                                  DEFER,
                cancelBackoffTimer();
                decreaseBackoffPeriod();
            );
        }
        FSMA_State(TRANSMIT)
        {
            FSMA_Enter(sendDataFrame(getCurrentTransmission()));
            FSMA_Event_Transition(Transmit-Broadcast,
                                  msg == endData && isBroadcast(getCurrentTransmission()),
                                  IDLE,
                finishCurrentTransmission();
                numSentBroadcast++;
            );
            FSMA_Event_Transition(Transmit-Unicast-No-Ack,
                                  msg == endData && !useAck && !isBroadcast(getCurrentTransmission()),
                                  IDLE,
                finishCurrentTransmission();
                numSent++;
            );
            FSMA_Event_Transition(Transmit-Unicast-Use-Ack,
                                  msg == endData && useAck && !isBroadcast(getCurrentTransmission()),
                                  WAITACK,
            );
        }
        FSMA_State(WAITACK)
        {
            FSMA_Enter(scheduleAckTimeout(getCurrentTransmission()));
            FSMA_Event_Transition(Receive-Ack,
                                  isLowerMessage(msg) && isFcsOk(frame) && isForUs(frame) && isAck(frame),
                                  IDLE,
                if (retryCounter == 0) numSentWithoutRetry++;
                numSent++;
                cancelAckTimer();
                finishCurrentTransmission();
            );
            FSMA_Event_Transition(Give-Up-Transmission,
                                  msg == endAckTimeout && retryCounter == retryLimit,
                                  IDLE,
                giveUpCurrentTransmission();
            );
            FSMA_Event_Transition(Retry-Transmission,
                                  msg == endAckTimeout,
                                  IDLE,
                retryCurrentTransmission();
            );
        }
        FSMA_State(RECEIVE)
        {
            FSMA_Event_Transition(Receive-Bit-Error,
                                  isLowerMessage(msg) && !isFcsOk(frame),
                                  IDLE,
                numCollision++;
                emitPacketDropSignal(frame, INCORRECTLY_RECEIVED);
            );
            FSMA_Event_Transition(Receive-Unexpected-Ack,
                                  isLowerMessage(msg) && isAck(frame),
                                  IDLE,
            );
            FSMA_Event_Transition(Receive-Broadcast,
                                  isLowerMessage(msg) && isBroadcast(frame),
                                  IDLE,
                decapsulate(frame);
                sendUp(frame);
                numReceivedBroadcast++;
            );
            FSMA_Event_Transition(Receive-Unicast-No-Ack,
                                  isLowerMessage(msg) && isForUs(frame) && !useAck,
                                  IDLE,
                decapsulate(frame);
                sendUp(frame);
                numReceived++;
            );
            FSMA_Event_Transition(Receive-Unicast-Use-Ack,
                                  isLowerMessage(msg) && isForUs(frame) && useAck,
                                  WAITSIFS,
                auto frameCopy = frame->dup();
                decapsulate(frameCopy);
                sendUp(frameCopy);
                numReceived++;
            );
            FSMA_Event_Transition(Receive-Unicast-Not-For-Us,
                                  isLowerMessage(msg) && !isForUs(frame),
                                  IDLE,
                emitPacketDropSignal(frame, NOT_ADDRESSED_TO_US, retryLimit);
            );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSifsTimer(frame));
            FSMA_Event_Transition(Transmit-Ack,
                                  msg == endSifs,
                                  IDLE,
                sendAckFrame();
            );
        }
    }
    if (fsm.getState() == IDLE) {
        if (isReceiving())
            handleWithFsm(mediumStateChange);
        else if (currentTxFrame != nullptr)
            handleWithFsm(currentTxFrame);
        else if (!txQueue->isEmpty()) {
            popTxQueue();
            handleWithFsm(currentTxFrame);
        }
    }
    if (isLowerMessage(msg) && frame->getOwner() == this && endSifs->getContextPointer() != frame)
        delete frame;
    getDisplayString().setTagArg("t", 0, fsm.getStateName());
}

void CsmaCaMac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal)
        handleWithFsm(mediumStateChange);
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(value);
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            handleWithFsm(endData);
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        }
        transmissionState = newRadioTransmissionState;
    }
}

void CsmaCaMac::encapsulate(Packet *frame)
{
    auto macHeader = makeShared<CsmaCaMacDataHeader>();
    macHeader->setChunkLength(headerLength);
    macHeader->setHeaderLengthField(B(headerLength).get());
    auto transportProtocol = frame->getTag<PacketProtocolTag>()->getProtocol();
    auto networkProtocol = ProtocolGroup::ethertype.getProtocolNumber(transportProtocol);
    macHeader->setNetworkProtocol(networkProtocol);
    macHeader->setTransmitterAddress(interfaceEntry->getMacAddress());
    macHeader->setReceiverAddress(frame->getTag<MacAddressReq>()->getDestAddress());
    auto userPriorityReq = frame->findTag<UserPriorityReq>();
    int userPriority = userPriorityReq == nullptr ? UP_BE : userPriorityReq->getUserPriority();
    macHeader->setPriority(userPriority == -1 ? UP_BE : userPriority);
    frame->insertAtFront(macHeader);
    auto macTrailer = makeShared<CsmaCaMacTrailer>();
    macTrailer->setFcsMode(fcsMode);
    if (fcsMode == FCS_COMPUTED)
        macTrailer->setFcs(computeFcs(frame->peekAllAsBytes()));
    frame->insertAtBack(macTrailer);
    auto macAddressInd = frame->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(macHeader->getTransmitterAddress());
    macAddressInd->setDestAddress(macHeader->getReceiverAddress());
    frame->getTag<PacketProtocolTag>()->setProtocol(&Protocol::csmaCaMac);
}

void CsmaCaMac::decapsulate(Packet *frame)
{
    auto macHeader = frame->popAtFront<CsmaCaMacDataHeader>();
    frame->popAtBack(B(4));
    auto addressInd = frame->addTagIfAbsent<MacAddressInd>();
    addressInd->setSrcAddress(macHeader->getTransmitterAddress());
    addressInd->setDestAddress(macHeader->getReceiverAddress());
    frame->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    frame->addTagIfAbsent<UserPriorityInd>()->setUserPriority(macHeader->getPriority());
    auto networkProtocol = macHeader->getNetworkProtocol();
    auto transportProtocol = ProtocolGroup::ethertype.getProtocol(networkProtocol);
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(transportProtocol);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(transportProtocol);
}

/****************************************************************
 * Timer functions.
 */
void CsmaCaMac::scheduleSifsTimer(Packet *frame)
{
    EV << "scheduling SIFS timer\n";
    endSifs->setContextPointer(frame);
    scheduleAfter(sifsTime, endSifs);
}

void CsmaCaMac::scheduleDifsTimer()
{
    EV << "scheduling DIFS timer\n";
    scheduleAfter(difsTime, endDifs);
}

void CsmaCaMac::cancelDifsTimer()
{
    EV << "canceling DIFS timer\n";
    cancelEvent(endDifs);
}

void CsmaCaMac::scheduleAckTimeout(Packet *frameToSend)
{
    EV << "scheduling ACK timeout\n";
    scheduleAfter(ackTimeout, endAckTimeout);
}

void CsmaCaMac::cancelAckTimer()
{
    EV << "canceling ACK timer\n";
    cancelEvent(endAckTimeout);
}

void CsmaCaMac::invalidateBackoffPeriod()
{
    backoffPeriod = -1;
}

bool CsmaCaMac::isInvalidBackoffPeriod()
{
    return backoffPeriod == -1;
}

void CsmaCaMac::generateBackoffPeriod()
{
    ASSERT(0 <= retryCounter && retryCounter <= retryLimit);
    EV << "generating backoff slot number for retry: " << retryCounter << endl;
    int cw;
    if (getCurrentTransmission()->peekAtFront<CsmaCaMacHeader>()->getReceiverAddress().isMulticast())
        cw = cwMulticast;
    else
        cw = std::min(cwMax, (cwMin + 1) * (1 << retryCounter) - 1);
    int slots = intrand(cw + 1);
    EV << "generated backoff slot number: " << slots << " , cw: " << cw << endl;
    backoffPeriod = slots * slotTime;
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period set to " << backoffPeriod << endl;
}

void CsmaCaMac::decreaseBackoffPeriod()
{
    simtime_t elapsedBackoffTime = simTime() - endBackoff->getSendingTime();
    backoffPeriod -= ((int)(elapsedBackoffTime / slotTime)) * slotTime;
    ASSERT(backoffPeriod >= 0);
    EV << "backoff period decreased to " << backoffPeriod << endl;
}

void CsmaCaMac::scheduleBackoffTimer()
{
    EV << "scheduling backoff timer\n";
    if (isInvalidBackoffPeriod())
        generateBackoffPeriod();
    scheduleAfter(backoffPeriod, endBackoff);
}

void CsmaCaMac::cancelBackoffTimer()
{
    EV << "canceling backoff timer\n";
    cancelEvent(endBackoff);
}

/****************************************************************
 * Frame sender functions.
 */
void CsmaCaMac::sendDataFrame(Packet *frameToSend)
{
    EV << "sending Data frame " << frameToSend->getName() << endl;
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(frameToSend->dup());
}

void CsmaCaMac::sendAckFrame()
{
    EV << "sending Ack frame\n";
    auto frameToAck = static_cast<Packet *>(endSifs->getContextPointer());
    endSifs->setContextPointer(nullptr);
    auto macHeader = makeShared<CsmaCaMacAckHeader>();
    macHeader->setChunkLength(ackLength);
    macHeader->setHeaderLengthField(B(ackLength).get());
    macHeader->setReceiverAddress(frameToAck->peekAtFront<CsmaCaMacHeader>()->getTransmitterAddress());
    auto frame = new Packet("CsmaAck");
    frame->insertAtFront(macHeader);
    auto macTrailer = makeShared<CsmaCaMacTrailer>();
    macTrailer->setFcsMode(fcsMode);
    if (fcsMode == FCS_COMPUTED)
        macTrailer->setFcs(computeFcs(frame->peekAllAsBytes()));
    frame->insertAtBack(macTrailer);
    auto macAddressInd = frame->addTag<MacAddressInd>();
    macAddressInd->setSrcAddress(macHeader->getTransmitterAddress());
    macAddressInd->setDestAddress(macHeader->getReceiverAddress());
    frame->addTag<PacketProtocolTag>()->setProtocol(&Protocol::csmaCaMac);
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(frame);
    delete frameToAck;
}

/****************************************************************
 * Helper functions.
 */
void CsmaCaMac::finishCurrentTransmission()
{
    deleteCurrentTxFrame();
    resetTransmissionVariables();
}

void CsmaCaMac::giveUpCurrentTransmission()
{
    auto packet = getCurrentTransmission();
    emitPacketDropSignal(packet, RETRY_LIMIT_REACHED, retryLimit);
    emit(linkBrokenSignal, packet);
    deleteCurrentTxFrame();
    resetTransmissionVariables();
    numGivenUp++;
}

void CsmaCaMac::retryCurrentTransmission()
{
    ASSERT(retryCounter < retryLimit);
    retryCounter++;
    numRetry++;
    generateBackoffPeriod();
}

Packet *CsmaCaMac::getCurrentTransmission()
{
    ASSERT(currentTxFrame != nullptr);
    return currentTxFrame;
}

void CsmaCaMac::resetTransmissionVariables()
{
    backoffPeriod = -1;
    retryCounter = 0;
}

void CsmaCaMac::emitPacketDropSignal(Packet *frame, PacketDropReason reason, int limit)
{
    PacketDropDetails details;
    details.setReason(reason);
    details.setLimit(limit);
    emit(packetDroppedSignal, frame, &details);
}

bool CsmaCaMac::isMediumFree()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
}

bool CsmaCaMac::isReceiving()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_RECEIVING;
}

bool CsmaCaMac::isAck(Packet *frame)
{
    const auto& macHeader = frame->peekAtFront<CsmaCaMacHeader>();
    return macHeader->getType() == CSMA_ACK;
}

bool CsmaCaMac::isBroadcast(Packet *frame)
{
    const auto& macHeader = frame->peekAtFront<CsmaCaMacHeader>();
    return macHeader->getReceiverAddress().isBroadcast();
}

bool CsmaCaMac::isForUs(Packet *frame)
{
    const auto& macHeader = frame->peekAtFront<CsmaCaMacHeader>();
    return macHeader->getReceiverAddress() == interfaceEntry->getMacAddress();
}

bool CsmaCaMac::isFcsOk(Packet *frame)
{
    if (frame->hasBitError() || !frame->peekData()->isCorrect())
        return false;
    else {
        const auto& trailer = frame->peekAtBack<CsmaCaMacTrailer>(B(4));
        switch (trailer->getFcsMode()) {
            case FCS_DECLARED_INCORRECT:
                return false;
            case FCS_DECLARED_CORRECT:
                return true;
            case FCS_COMPUTED: {
                const auto& fcsBytes = frame->peekDataAt<BytesChunk>(B(0), frame->getDataLength() - trailer->getChunkLength());
                auto bufferLength = B(fcsBytes->getChunkLength()).get();
                auto buffer = new uint8_t[bufferLength];
                fcsBytes->copyToBuffer(buffer, bufferLength);
                auto computedFcs = ethernetCRC(buffer, bufferLength);
                delete [] buffer;
                return computedFcs == trailer->getFcs();
            }
            default:
                throw cRuntimeError("Unknown FCS mode");
        }
    }
}

uint32_t CsmaCaMac::computeFcs(const Ptr<const BytesChunk>& bytes)
{
    auto bufferLength = B(bytes->getChunkLength()).get();
    auto buffer = new uint8_t[bufferLength];
    bytes->copyToBuffer(buffer, bufferLength);
    auto computedFcs = ethernetCRC(buffer, bufferLength);
    delete [] buffer;
    return computedFcs;
}

void CsmaCaMac::handleStopOperation(LifecycleOperation *operation)
{
    MacProtocolBase::handleStopOperation(operation);
    resetTransmissionVariables();
}

void CsmaCaMac::handleCrashOperation(LifecycleOperation *operation)
{
    MacProtocolBase::handleCrashOperation(operation);
    resetTransmissionVariables();
}

} // namespace inet
