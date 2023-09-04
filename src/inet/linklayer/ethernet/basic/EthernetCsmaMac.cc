//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/basic/EthernetCsmaMac.h"

#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyConstants.h"

namespace inet {

Define_Module(EthernetCsmaMac);

simsignal_t EthernetCsmaMac::carrierSenseChangedSignal = cComponent::registerSignal("carrierSenseChanged");
simsignal_t EthernetCsmaMac::collisionChangedSignal = cComponent::registerSignal("collisionChanged");
simsignal_t EthernetCsmaMac::stateChangedSignal = cComponent::registerSignal("stateChanged");

Register_Enum(EthernetCsmaMac::State,
    (EthernetCsmaMac::IDLE,
     EthernetCsmaMac::WAIT_IFG,
     EthernetCsmaMac::TRANSMITTING,
     EthernetCsmaMac::JAMMING,
     EthernetCsmaMac::BACKOFF,
     EthernetCsmaMac::RECEIVING));

EthernetCsmaMac::~EthernetCsmaMac()
{
    cancelAndDelete(txTimer);
    cancelAndDelete(ifgTimer);
    cancelAndDelete(jamTimer);
    cancelAndDelete(backoffTimer);
}

void EthernetCsmaMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        fcsMode = parseFcsMode(par("fcsMode"));
        txQueue = getQueue(gate(upperLayerInGateId));
        sendRawBytes = par("sendRawBytes");
        promiscuous = par("promiscuous");
        phy = getConnectedModule<IEthernetCsmaPhy>(gate("lowerLayerOut"));
        txTimer = new cMessage("TxTimer", END_TX_TIMER);
        ifgTimer = new cMessage("IfgTimer", END_IFG_TIMER);
        jamTimer = new cMessage("JamTimer", END_JAM_TIMER);
        backoffTimer = new cMessage("BackoffTimer", END_BACKOFF_TIMER);
        fsm.setStateChangedSignal(stateChangedSignal);
        fsm.setState(IDLE, "IDLE");
        emit(carrierSenseChangedSignal, (int)carrierSense);
        emit(collisionChangedSignal, (int)collision);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        // TODO register to networkInterface parameter change signal and process changes
        mode = &EthernetModes::getEthernetMode(networkInterface->getDatarate());
    }
}

void EthernetCsmaMac::finish()
{
    emit(carrierSenseChangedSignal, (int)carrierSense);
    emit(collisionChangedSignal, (int)collision);
    emit(stateChangedSignal, fsm.getState());
}

void EthernetCsmaMac::refreshDisplay() const
{
    auto& displayString = getDisplayString();
    std::stringstream stream;
    stream << fsm.getStateName();
    displayString.setTagArg("t", 0, stream.str().c_str());
}

void EthernetCsmaMac::configureNetworkInterface()
{
    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    networkInterface->setMtu(par("mtu"));

    // capabilities
    networkInterface->setMulticast(true);
    networkInterface->setBroadcast(true);
}

void EthernetCsmaMac::handleSelfMessage(cMessage *message)
{
    handleWithFsm(message->getKind(), message);
}

void EthernetCsmaMac::handleUpperPacket(Packet *packet)
{
    EV_DEBUG << "Handling upper packet" << EV_FIELD(packet) << EV_ENDL;
    handleWithFsm(UPPER_PACKET, packet);
}

void EthernetCsmaMac::handleLowerPacket(Packet *packet)
{
    EV_DEBUG << "Handling lower packet" << EV_FIELD(packet) << EV_ENDL;
    handleWithFsm(LOWER_PACKET, packet);
}

void EthernetCsmaMac::handleCarrierSenseStart()
{
    Enter_Method("handleCarrierSenseStart");
    ASSERT(!carrierSense);
    EV_DEBUG << "Handling carrier sense start" << EV_ENDL;
    carrierSense = true;
    emit(carrierSenseChangedSignal, (int)carrierSense);
    handleWithFsm(CARRIER_SENSE_START, nullptr);
}

void EthernetCsmaMac::handleCarrierSenseEnd()
{
    Enter_Method("handleCarrierSenseEnd");
    ASSERT(carrierSense);
    EV_DEBUG << "Handling carrier sense end" << EV_ENDL;
    carrierSense = false;
    emit(carrierSenseChangedSignal, (int)carrierSense);
    handleWithFsm(CARRIER_SENSE_END, nullptr);
}

void EthernetCsmaMac::handleCollisionStart()
{
    Enter_Method("handleCollisionStart");
    ASSERT(!collision);
    EV_DEBUG << "Handling collision start" << EV_ENDL;
    collision = true;
    emit(collisionChangedSignal, (int)collision);
    handleWithFsm(COLLISION_START, nullptr);
}

void EthernetCsmaMac::handleCollisionEnd()
{
    Enter_Method("handleCollisionEnd");
    ASSERT(collision);
    EV_DEBUG << "Handling collision end" << EV_ENDL;
    collision = false;
    emit(collisionChangedSignal, (int)collision);
    // NOTE: this event is not needed in the FSM
}

void EthernetCsmaMac::handleReceptionStart(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionStart");
    EV_DEBUG << "Handling reception start" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (packet != nullptr)
        take(packet);
    delete packet;
    // NOTE: this event is not needed in the FSM
}

void EthernetCsmaMac::handleReceptionError(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionError");
    EV_DEBUG << "Handling reception error" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (packet != nullptr)
        take(packet);
    delete packet;
    // NOTE: this event is not needed in the FSM
}

void EthernetCsmaMac::handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("handleReceptionEnd");
    EV_DEBUG << "Handling reception end" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (packet != nullptr)
        take(packet);
    if (signalType == DATA)
        handleWithFsm(LOWER_PACKET, packet);
    else if (signalType == JAM)
        ASSERT(packet == nullptr);
    else
        throw cRuntimeError("Unknown signal type");
}

void EthernetCsmaMac::handleWithFsm(int event, cMessage *message)
{
    { FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(ASSERT(!carrierSense));
            FSMA_Event_Transition(UPPER_PACKET,
                                  event == UPPER_PACKET,
                                  TRANSMITTING,
                setCurrentTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Event_Transition(CRS_START,
                                  event == CARRIER_SENSE_START,
                                  RECEIVING,
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAIT_IFG) {
            FSMA_Enter(scheduleIfgTimer());
            FSMA_Event_Transition(IFG_END_AND_HAS_CURRENT_TX,
                                  event == END_IFG_TIMER && currentTxFrame != nullptr,
                                  TRANSMITTING,
            );
            FSMA_Event_Transition(IFG_END_AND_QUEUE_NOT_EMPTY,
                                  event == END_IFG_TIMER && currentTxFrame == nullptr && !txQueue->isEmpty(),
                                  TRANSMITTING,
                setCurrentTransmission(txQueue->dequeuePacket());
            );
            FSMA_Event_Transition(IFG_END_AND_QUEUE_EMPTY_AND_CRS,
                                  event == END_IFG_TIMER && currentTxFrame == nullptr && txQueue->isEmpty() && carrierSense,
                                  RECEIVING,
            );
            FSMA_Event_Transition(IFG_END_AND_QUEUE_EMPTY_AND_NO_CRS,
                                  event == END_IFG_TIMER && currentTxFrame == nullptr && txQueue->isEmpty() && !carrierSense,
                                  IDLE,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(TRANSMITTING) {
            FSMA_Enter(
                startTransmission();
                FSMA_Delay_Action(phy->startFrameTransmission(currentTxFrame->dup(), ESDNONE));
            );
            FSMA_Event_Transition(TX_END,
                                  event == END_TX_TIMER,
                                  WAIT_IFG,
                endTransmission();
                FSMA_Delay_Action(phy->endFrameTransmission());
            );
            FSMA_Event_Transition(COLLISION_START,
                                  event == COLLISION_START,
                                  JAMMING,
                abortTransmission();
                FSMA_Delay_Action(phy->endFrameTransmission());
                FSMA_Delay_Action(phy->startSignalTransmission(JAM));
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(JAMMING) {
            FSMA_Enter(scheduleJamTimer());
            FSMA_Event_Transition(JAM_END,
                                  event == END_JAM_TIMER,
                                  BACKOFF,
                FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                retryTransmission();
                scheduleBackoffTimer();
            );
            FSMA_Event_Transition(LOWER_PACKET,
                                  event == LOWER_PACKET,
                                  JAMMING,
                auto packet = check_and_cast<Packet *>(message);
                PacketDropDetails details;
                details.setReason(INCORRECTLY_RECEIVED);
                emit(packetDroppedSignal, packet, &details);
                delete packet
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(BACKOFF) {
            FSMA_Event_Transition(BACKOFF_END_AND_NO_CRS,
                                  event == END_BACKOFF_TIMER && !carrierSense,
                                  WAIT_IFG,
            );
            FSMA_Event_Transition(BACKOFF_END_AND_CRS,
                                  event == END_BACKOFF_TIMER && carrierSense,
                                  RECEIVING,
            );
            FSMA_Event_Transition(LOWER_PACKET,
                                  event == LOWER_PACKET,
                                  BACKOFF,
                processReceivedFrame(check_and_cast<Packet *>(message));
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(RECEIVING) {
            FSMA_Event_Transition(LOWER_PACKET,
                                  event == LOWER_PACKET,
                                  RECEIVING,
                processReceivedFrame(check_and_cast<Packet *>(message));
            );
            FSMA_Event_Transition(CRS_END,
                                  event == CARRIER_SENSE_END,
                                  WAIT_IFG,
            );
            FSMA_Fail_On_Unhandled_Event();
        }
    } }
    fsm.executeDelayedActions();
}

void EthernetCsmaMac::setCurrentTransmission(Packet *packet)
{
    ASSERT(currentTxFrame == nullptr);
    currentTxFrame = packet;
}

void EthernetCsmaMac::startTransmission()
{
    EV_DEBUG << "Starting frame transmission" << EV_FIELD(currentTxFrame) << EV_ENDL;
    MacAddress address = getMacAddress();
    const auto& macHeader = currentTxFrame->peekAtFront<EthernetMacHeader>();
    if (macHeader->getDest().equals(address)) {
        throw cRuntimeError("Logic error: frame %s from higher layer has local MAC address as dest (%s)",
                currentTxFrame->getFullName(), macHeader->getDest().str().c_str());
    }
    if (currentTxFrame->getDataLength() > MAX_ETHERNET_FRAME_BYTES) {
        throw cRuntimeError("Packet length from higher layer (%s) exceeds maximum Ethernet frame size (%s)",
                currentTxFrame->getDataLength().str().c_str(), MAX_ETHERNET_FRAME_BYTES.str().c_str());
    }
    addPaddingAndSetFcs(currentTxFrame, MIN_ETHERNET_FRAME_BYTES);
    scheduleTxTimer(currentTxFrame);
}

void EthernetCsmaMac::endTransmission()
{
    EV_DEBUG << "Ending frame transmission" << EV_FIELD(currentTxFrame) << EV_ENDL;
    delete currentTxFrame;
    currentTxFrame = nullptr;
    numRetries = 0;
}

void EthernetCsmaMac::abortTransmission()
{
    EV_DEBUG << "Aborting frame transmission" << EV_FIELD(currentTxFrame) << EV_ENDL;
    cancelEvent(txTimer);
}

void EthernetCsmaMac::retryTransmission()
{
    EV_DEBUG << "Retrying frame transmission" << EV_FIELD(currentTxFrame) << EV_ENDL;
    if (++numRetries > MAX_ATTEMPTS) {
        EV_DEBUG << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(MAX_ATTEMPTS);
        dropCurrentTxFrame(details);
        numRetries = 0;
    }
}

void EthernetCsmaMac::processReceivedFrame(Packet *packet)
{
    EV_DEBUG << "Processing received frame" << EV_FIELD(packet) << EV_ENDL;
    if (packet->hasBitError()) { // TODO check FCS
        EV_WARN << "Frame " << packet->getName() << " received with errors, discarding\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
    else {
        const auto& header = packet->peekAtFront<EthernetMacHeader>();
        auto macAddressInd = packet->addTag<MacAddressInd>();
        macAddressInd->setSrcAddress(header->getSrc());
        macAddressInd->setDestAddress(header->getDest());
        if (isFrameNotForUs(header)) {
            EV_WARN << "Frame " << packet->getName() << " not destined to us, discarding\n";
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
        else {
            packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
            if (networkInterface)
                packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
            sendUp(packet);
        }
    }
}

bool EthernetCsmaMac::isFrameNotForUs(const Ptr<const EthernetMacHeader>& header)
{
    // Current ethernet mac implementation does not support the configuration of multicast
    // ethernet address groups. We rather accept all multicast frames (just like they were
    // broadcasts) and pass it up to the higher layer where they will be dropped
    // if not needed.
    //
    // PAUSE frames must be handled a bit differently because they are processed at
    // this level. Multicast PAUSE frames should not be processed unless they have a
    // destination of MULTICAST_PAUSE_ADDRESS. We drop all PAUSE frames that have a
    // different muticast destination. (Note: Would the multicast ethernet addressing
    // implemented, we could also process the PAUSE frames destined to any of our
    // multicast adresses)
    // All NON-PAUSE frames must be passed to the upper layer if the interface is
    // in promiscuous mode.

    if (header->getDest().equals(getMacAddress()))
        return false;

    if (header->getDest().isBroadcast())
        return false;

    bool isPause = (header->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL);

    if (!isPause && (promiscuous || header->getDest().isMulticast()))
        return false;

    if (isPause && header->getDest().equals(MacAddress::MULTICAST_PAUSE_ADDRESS))
        return false;

    return true;
}

IPassivePacketSource *EthernetCsmaMac::getProvider(const cGate *gate)
{
    return gate->getId() == upperLayerInGateId ? txQueue.get() : nullptr;
}

void EthernetCsmaMac::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (!carrierSense && currentTxFrame == nullptr && fsm.getState() == IDLE && canDequeuePacket()) {
        Packet *packet = dequeuePacket();
        handleUpperPacket(packet);
    }
}

void EthernetCsmaMac::scheduleTxTimer(Packet *packet)
{
    EV_DEBUG << "Scheduling TX timer" << EV_FIELD(packet) << EV_ENDL;
    simtime_t duration = b(packet->getDataLength() + ETHERNET_PHY_HEADER_LEN + phy->getEsdLength()).get() / mode->bitrate;
    scheduleAfter(duration, txTimer);
}

void EthernetCsmaMac::scheduleIfgTimer()
{
    EV_DEBUG << "Scheduling IFG timer" << EV_ENDL;
    simtime_t duration = b(INTERFRAME_GAP_BITS).get() / mode->bitrate;
    scheduleAfter(duration, ifgTimer);
}

void EthernetCsmaMac::scheduleJamTimer()
{
    EV_DEBUG << "Scheduling jam timer" << EV_ENDL;
    simtime_t duration = b(JAM_SIGNAL_BYTES).get() / mode->bitrate;
    scheduleAfter(duration, jamTimer);
}

void EthernetCsmaMac::scheduleBackoffTimer()
{
    EV_DEBUG << "Scheduling backoff timer" << EV_ENDL;
    int backoffRange = (numRetries >= BACKOFF_RANGE_LIMIT) ? 1024 : (1 << numRetries);
    int slotNumber = intuniform(0, backoffRange - 1);
    EV_DEBUG << "Executing backoff procedure" << EV_FIELD(slotNumber) << ", backoffRange = [0," << backoffRange - 1 << "]" << EV_ENDL;
    simtime_t backoffDuration = slotNumber * 512 / mode->bitrate;
    scheduleAfter(backoffDuration, backoffTimer);
}

void EthernetCsmaMac::addPaddingAndSetFcs(Packet *packet, B requiredMinBytes) const
{
    auto ethFcs = packet->removeAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    ethFcs->setFcsMode(fcsMode);

    B paddingLength = requiredMinBytes - ETHER_FCS_BYTES - B(packet->getByteLength());
    if (paddingLength > B(0)) {
        const auto& ethPadding = makeShared<EthernetPadding>();
        ethPadding->setChunkLength(paddingLength);
        packet->insertAtBack(ethPadding);
    }

    switch (ethFcs->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            ethFcs->setFcs(0xC00DC00DL);
            break;
        case FCS_DECLARED_INCORRECT:
            ethFcs->setFcs(0xBAADBAADL);
            break;
        case FCS_COMPUTED: { // calculate FCS
            auto ethBytes = packet->peekDataAsBytes();
            auto bufferLength = B(ethBytes->getChunkLength()).get();
            auto buffer = new uint8_t[bufferLength];
            // 1. fill in the data
            ethBytes->copyToBuffer(buffer, bufferLength);
            // 2. compute the FCS
            auto computedFcs = ethernetCRC(buffer, bufferLength);
            delete[] buffer;
            ethFcs->setFcs(computedFcs);
            break;
        }
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)(ethFcs->getFcsMode()));
    }

    packet->insertAtBack(ethFcs);
}

} // namespace inet

