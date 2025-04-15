//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetCsmaPhy.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/PacketEventTag.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetCsmaPhy);

simsignal_t EthernetCsmaPhy::stateChangedSignal = cComponent::registerSignal("stateChanged");
simsignal_t EthernetCsmaPhy::receivedSignalTypeSignal = cComponent::registerSignal("receivedSignalType");
simsignal_t EthernetCsmaPhy::transmittedSignalTypeSignal = cComponent::registerSignal("transmittedSignalType");
simsignal_t EthernetCsmaPhy::busUsedSignal = cComponent::registerSignal("busUsed");

Register_Enum(EthernetCsmaPhy::State,
    (EthernetCsmaPhy::IDLE,
     EthernetCsmaPhy::TRANSMITTING,
     EthernetCsmaPhy::RECEIVING,
     EthernetCsmaPhy::COLLISION,
     EthernetCsmaPhy::CRS_ON));

EthernetCsmaPhy::~EthernetCsmaPhy()
{
    cancelAndDelete(rxEndTimer);
    cancelAndDelete(crsOffTimer);
    delete currentTxSignal;
    currentTxSignal = nullptr;
    for (auto rxSignal : rxSignals)
        delete rxSignal.signal;
    rxSignals.clear();
}

void EthernetCsmaPhy::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physInGate->setDeliverImmediately(true);
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        upperLayerOutGate = gate("upperLayerOut");
        mac = getConnectedModule<IEthernetCsmaMac>(gate("upperLayerOut"));
        rxEndTimer = new cMessage("RxEndTimer", RX_END_TIMER);
        rxEndTimer->setSchedulingPriority(SHRT_MIN);
        crsOffTimer = new cMessage("CrsOffTimer", CRS_OFF_TIMER);
        crsOffTimer->setSchedulingPriority(SHRT_MAX);
        fsm.setStateChangedSignal(stateChangedSignal);
        fsm.setState(IDLE, "IDLE");
        setTxUpdateSupport(true);
        emit(receivedSignalTypeSignal, NONE);
        emit(transmittedSignalTypeSignal, NONE);
        emit(busUsedSignal, 0);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        networkInterface = getContainingNicModule(this);
        // TODO register to networkInterface parameter change signal and process changes
        mode = EthernetModes::getEthernetMode(networkInterface->getDatarate());
    }
}

void EthernetCsmaPhy::finish()
{
    emit(stateChangedSignal, fsm.getState());
    emit(receivedSignalTypeSignal, currentRxSignal != nullptr ? currentRxSignal->getKind() : NONE);
    emit(transmittedSignalTypeSignal, currentTxSignal != nullptr ? currentTxSignal->getKind() : NONE);
    emit(busUsedSignal, (currentRxSignal != nullptr && currentRxSignal->getKind() == DATA) || (currentTxSignal != nullptr && currentTxSignal->getKind() == DATA) ? 1 : 0);
}

void EthernetCsmaPhy::refreshDisplay() const
{
    auto& displayString = getDisplayString();
    std::stringstream stream;
    stream << fsm.getStateName();
    displayString.setTagArg("t", 0, stream.str().c_str());
}

void EthernetCsmaPhy::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerInGate)
        handleUpperPacket(check_and_cast<Packet *>(message));
    else if (message->getArrivalGate() == physInGate)
        handleEthernetSignal(check_and_cast<EthernetSignalBase *>(message));
    else
        throw cRuntimeError("Received unknown message");
}

void EthernetCsmaPhy::handleSelfMessage(cMessage *message)
{
    EV_DEBUG << "Handling self message" << EV_FIELD(message) << EV_ENDL;
    handleWithFsm(message->getKind(), message);
}

void EthernetCsmaPhy::handleUpperPacket(Packet *packet)
{
    EV_DEBUG << "Handling upper packet" << EV_FIELD(packet) << EV_ENDL;
    startFrameTransmission(packet, ESD);
}

void EthernetCsmaPhy::handleEthernetSignal(EthernetSignalBase *signal)
{
    EV_DEBUG << "Handling Ethernet signal" << EV_FIELD(signal) << EV_ENDL;
    handleWithFsm(signal->isUpdate() ? RX_UPDATE : RX_START, signal);
}

void EthernetCsmaPhy::handleWithFsm(int event, cMessage *message)
{
    { FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(ASSERT(!crsOffTimer->isScheduled() && !rxEndTimer->isScheduled()))
            FSMA_Event_Transition(TX_START,
                                  event == TX_START,
                                  TRANSMITTING,
                handleStartTransmission(check_and_cast<EthernetSignalBase *>(message));
                FSMA_Delay_Action(mac->handleCarrierSenseStart());
            );
            FSMA_Event_Transition(RX_START,
                                  event == RX_START,
                                  RECEIVING,
                handleStartReception(check_and_cast<EthernetSignalBase *>(message));
                FSMA_Delay_Action(mac->handleCarrierSenseStart());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(TRANSMITTING) {
            FSMA_Event_Transition(TX_END,
                                  event == TX_END,
                                  CRS_ON,
                handleEndTransmission();
            );
            FSMA_Event_Transition(RX_START,
                                  event == RX_START,
                                  COLLISION,
                handleStartReception(check_and_cast<EthernetSignalBase *>(message));
                FSMA_Delay_Action(mac->handleCollisionStart());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(RECEIVING) {
            FSMA_Event_Transition(RX_START,
                                  event == RX_START,
                                  RECEIVING,
                updateRxSignals(check_and_cast<EthernetSignalBase *>(message));
                // TODO mac->handleReceptionError
            );
            FSMA_Event_Transition(RX_UPDATE,
                                  event == RX_UPDATE,
                                  RECEIVING,
                updateRxSignals(check_and_cast<EthernetSignalBase *>(message));
               // TODO mac->handleReceptionEnd or mac->handleReceptionError?
            );
            FSMA_Event_Transition(RX_END,
                                  event == RX_END_TIMER,
                                  CRS_ON,
                handleEndReception();
            );
            FSMA_Event_Transition(TX_START,
                                  event == TX_START,
                                  COLLISION,
                handleStartTransmission(check_and_cast<EthernetSignalBase *>(message));
                FSMA_Delay_Action(mac->handleCollisionStart());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(COLLISION) {
            FSMA_Event_Transition(TX_START_JAM,
                                  event == TX_START && message->getKind() == JAM,
                                  COLLISION,
                handleStartTransmission(check_and_cast<EthernetSignalBase *>(message));
            );
            FSMA_Event_Transition(TX_END_AND_NO_RX,
                                  event == TX_END && rxSignals.empty(),
                                  CRS_ON,
                handleEndTransmission();
                FSMA_Delay_Action(mac->handleCollisionEnd());
            );
            FSMA_Event_Transition(TX_END_AND_RX,
                                  event == TX_END && !rxSignals.empty(),
                                  COLLISION,
                handleEndTransmission();
            );
            FSMA_Event_Transition(RX_START,
                                  event == RX_START,
                                  COLLISION,
                updateRxSignals(check_and_cast<EthernetSignalBase *>(message));
            );
            FSMA_Event_Transition(RX_UPDATE,
                                  event == RX_UPDATE,
                                  COLLISION,
                updateRxSignals(check_and_cast<EthernetSignalBase *>(message));
            );
            FSMA_Event_Transition(RX_END_AND_TX,
                                  event == RX_END_TIMER && currentTxSignal != nullptr,
                                  COLLISION,
                handleEndReception();
            );
            FSMA_Event_Transition(RX_END_AND_NO_TX,
                                  event == RX_END_TIMER && currentTxSignal == nullptr,
                                  CRS_ON,
                handleEndReception();
                FSMA_Delay_Action(mac->handleCollisionEnd());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(CRS_ON) {
            FSMA_Enter(ASSERT(crsOffTimer->isScheduled()));
            FSMA_Event_Transition(TX_START,
                                  event == TX_START,
                                  TRANSMITTING,
                handleStartTransmission(check_and_cast<EthernetSignalBase *>(message));
            );
            FSMA_Event_Transition(RX_START,
                                  event == RX_START,
                                  RECEIVING,
                handleStartReception(check_and_cast<EthernetSignalBase *>(message));
            );
            FSMA_Event_Transition(CRS_OFF,
                                  event == CRS_OFF_TIMER,
                                  IDLE,
                FSMA_Delay_Action(mac->handleCarrierSenseEnd());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
    } }
    fsm.executeDelayedActions();
}

void EthernetCsmaPhy::startBeaconSignalTransmission()
{
    Enter_Method("startBeaconSignalTransmission");
    EV_DEBUG << "Starting beacon signal transmission" << EV_ENDL;
    auto signal = new EthernetSignal("Beacon");
    signal->setKind(BEACON);
    signal->setBitLength(20);
    handleWithFsm(TX_START, signal);
}

void EthernetCsmaPhy::startCommitSignalTransmission()
{
    Enter_Method("startCommitSignalTransmission");
    EV_DEBUG << "Starting commit signal transmission" << EV_ENDL;
    auto signal = new EthernetSignal("Commit");
    signal->setKind(COMMIT);
    signal->setEsd1(ESD);
    signal->setEsd2(ESDOK);
    // TODO: make it indefinite long?
    signal->setBitLength(512 + 32 + 96); // backoff + jam + ifg
    handleWithFsm(TX_START, signal);
}

void EthernetCsmaPhy::startJamSignalTransmission()
{
    Enter_Method("transmitJamSignal");
    EV_DEBUG << "Starting jam signal transmission" << EV_ENDL;
    auto signal = new EthernetSignal("Jam");
    signal->setKind(JAM);
    signal->setByteLength(JAM_SIGNAL_BYTES.get<B>());
    handleWithFsm(TX_START, signal);
}

void EthernetCsmaPhy::startSignalTransmission(EthernetSignalType signalType)
{
    ASSERT(!currentTxSignal);
    switch (signalType) {
        case BEACON: startBeaconSignalTransmission(); break;
        case COMMIT: startCommitSignalTransmission(); break;
        case JAM: startJamSignalTransmission(); break;
        default: throw cRuntimeError("Unknown signal type");
    }
}

void EthernetCsmaPhy::endSignalTransmission(EthernetEsdType esd1)
{
    Enter_Method("endSignalTransmission");
    ASSERT(currentTxSignal);
    ASSERT(currentTxSignal->getKind() != DATA);
    EV_DEBUG << "Ending signal transmission" << EV_ENDL;
    currentTxSignal->setEsd1(esd1);
    currentTxSignal->setEsd2(esd1 == ESDNONE ? ESDNONE : ESDOK);
    handleWithFsm(TX_END, currentTxSignal);
}

void EthernetCsmaPhy::startFrameTransmission(Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("startFrameTransmission");
    ASSERT(!currentTxSignal);
    EV_DEBUG << "Starting frame transmission" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    packet->clearTags();
    encapsulate(packet);
    auto signal = new EthernetSignal(packet->getName());
    signal->setKind(DATA);
    signal->setEsd1(esd1);
    signal->setEsd2(esd1 == ESDNONE ? ESDNONE : ESDOK);
    signal->encapsulate(packet);
    handleWithFsm(TX_START, signal);
}

void EthernetCsmaPhy::endFrameTransmission()
{
    Enter_Method("endFrameTransmission");
    ASSERT(currentTxSignal);
    ASSERT(currentTxSignal->getKind() == DATA);
    EV_DEBUG << "Ending frame transmission" << EV_ENDL;
    handleWithFsm(TX_END, currentTxSignal);
}

void EthernetCsmaPhy::handleStartTransmission(EthernetSignalBase *signal)
{
    ASSERT(currentTxSignal == nullptr);
    EV_DEBUG << "Handling start transmission" << EV_FIELD(signal) << EV_ENDL;
    simtime_t dataDuration = signal->getBitLength() / mode.bitrate;
    simtime_t esdDuration = signal->getEsd1() != ESDNONE || signal->getEsd2() != ESDNONE ? 8 / mode.bitrate : 0;
    simtime_t duration = dataDuration + esdDuration;
    signal->setSrcMacFullDuplex(false);
    signal->setBitrate(mode.bitrate);
    signal->setDuration(duration);
    currentTxSignal = signal;
    txEndTime = simTime() + duration;
    emit(transmissionStartedSignal, currentTxSignal);
    emit(transmittedSignalTypeSignal, currentTxSignal->getKind());
    emit(busUsedSignal, currentTxSignal->getKind() == DATA ? 1 : 0);
    send(signal->dup(), SendOptions().transmissionId(signal->getId()).duration(duration), physOutGate);
    scheduleCrsOffTimer();
}

void EthernetCsmaPhy::handleEndTransmission()
{
    ASSERT(currentTxSignal != nullptr);
    EV_DEBUG << "Handling end transmission" << EV_ENDL;
    emit(transmittedSignalTypeSignal, NONE);
    emit(busUsedSignal, 0);
    simtime_t duration = simTime() - currentTxSignal->getCreationTime(); // TODO save and use start tx time
    if (duration != currentTxSignal->getDuration()) {
        truncateSignal(currentTxSignal, duration); // TODO save and use start tx time
        emit(transmissionEndedSignal, currentTxSignal);
        send(currentTxSignal, SendOptions().finishTx(currentTxSignal->getId()), physOutGate);
    }
    else {
        emit(transmissionEndedSignal, currentTxSignal);
        delete currentTxSignal;
    }
    txEndTime = simTime();
    scheduleCrsOffTimer();
    currentTxSignal = nullptr;
    txEndTime = -1;
}

void EthernetCsmaPhy::handleStartReception(EthernetSignalBase *signal)
{
    EV_DEBUG << "Handling start reception" << EV_FIELD(signal) << EV_ENDL;
    currentRxSignal = signal;
    emit(receptionStartedSignal, signal);
    emit(receivedSignalTypeSignal, signal->getKind());
    emit(busUsedSignal, signal->getKind() == DATA ? 1 : 0);
    auto packet = check_and_cast_nullable<Packet *>(signal->getEncapsulatedPacket());
    auto signalType = static_cast<EthernetSignalType>(signal->getKind());
    if (fsm.getState() == IDLE || fsm.getState() == CRS_ON)
        fsm.insertDelayedAction([=] () { mac->handleReceptionStart(signalType, packet != nullptr ? packet->dup() : nullptr); });
    updateRxSignals(signal);
}

void EthernetCsmaPhy::handleEndReception()
{
    if (fsm.getState() == RECEIVING && rxSignals.size() == 1) {
        EthernetSignalBase *signal = rxSignals[0].signal;
        EV_DEBUG << "Handling end reception" << EV_FIELD(signal) << EV_ENDL;
        emit(receptionEndedSignal, signal);
        auto packet = check_and_cast_nullable<Packet *>(signal->decapsulate());
        auto signalType = static_cast<EthernetSignalType>(signal->getKind());
        if (!signal->hasBitError() && signalType == DATA)
            decapsulate(packet);
        if (packet != nullptr)
            packet->setBitError(signal->hasBitError());
        auto esd1 = signal->getEsd1();
        fsm.insertDelayedAction([=] () { mac->handleReceptionEnd(signalType, packet, esd1); });
        delete signal;
    }
    else {
        EV_DEBUG << "Handling end all receptions" << EV_ENDL;
        // TODO cannot emit nullptr but there's no right signal
        // emit(receptionEndedSignal, static_cast<EthernetSignalBase *>(nullptr));
        for (auto rxSignal : rxSignals)
            delete rxSignal.signal;
    }
    currentRxSignal = nullptr;
    emit(receivedSignalTypeSignal, NONE);
    emit(busUsedSignal, 0);
    rxSignals.clear();
}

void EthernetCsmaPhy::encapsulate(Packet *packet)
{
    EV_DEBUG << "Encapsulating packet" << EV_FIELD(packet) << EV_ENDL;
    auto phyHeader = makeShared<EthernetPhyHeader>();
    packet->insertAtFront(phyHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
    auto packetEvent = new PacketTransmittedEvent();
    simtime_t packetTransmissionTime = packet->getBitLength() / mode.bitrate;
    simtime_t bitTransmissionTime = packet->getBitLength() != 0 ? 1 / mode.bitrate : 0;
    packetEvent->setDatarate(bps(mode.bitrate));
    insertPacketEvent(this, packet, PEK_TRANSMITTED, bitTransmissionTime, 8 * bitTransmissionTime, packetEvent);
    increaseTimeTag<TransmissionTimeTag>(packet, bitTransmissionTime, packetTransmissionTime);
    if (auto channel = dynamic_cast<cDatarateChannel *>(physOutGate->findTransmissionChannel())) {
        insertPacketEvent(this, packet, PEK_PROPAGATED, 0, channel->getDelay());
        increaseTimeTag<PropagationTimeTag>(packet, channel->getDelay(), channel->getDelay());
    }
}

void EthernetCsmaPhy::decapsulate(Packet *packet)
{
    EV_DEBUG << "Decapsulating packet" << EV_FIELD(packet) << EV_ENDL;
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    ASSERT(packet->getDataLength() >= MIN_ETHERNET_FRAME_BYTES);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
}

void EthernetCsmaPhy::truncateSignal(EthernetSignalBase *signal, simtime_t duration)
{
    ASSERT(duration <= signal->getDuration());
    if (duration == signal->getDuration())
        return;
    int64_t newBitLength = duration.dbl() * signal->getBitrate();
    if (auto packet = check_and_cast_nullable<Packet *>(signal->decapsulate())) {
        // TODO removed length calculation based on the PHY layer (parallel bits, bit order, etc.)
        if (newBitLength < packet->getBitLength()) {
            packet->trimFront();
            packet->setBackOffset(B(newBitLength / 8)); //TODO rounded down to byte align instead of b(newBitLength)
            packet->trimBack();
            packet->setBitError(true);
        }
        signal->encapsulate(packet);
    }
    signal->setBitError(true);
    signal->setBitLength(newBitLength);
    signal->setDuration(duration);
}

void EthernetCsmaPhy::updateRxSignals(EthernetSignalBase *signal)
{
    simtime_t rxEndTime = simTime() + signal->getRemainingDuration();
    if (!signal->isUpdate())
        rxSignals.push_back(RxSignal(signal, rxEndTime));
    else {
        bool found = false;
        for (auto& rx : rxSignals) {
            if (rx.signal->getTransmissionId() == signal->getTransmissionId()) {
                delete rx.signal;
                rx.signal = signal;
                rx.rxEndTime = rxEndTime;
                found = true;
                break;
            }
        }
        if (!found)
            throw cRuntimeError("Cannot find signal");
    }
    scheduleRxEndTimer();
    scheduleCrsOffTimer();
}

simtime_t EthernetCsmaPhy::getMaxRxEndTime()
{
    simtime_t maxRxEndTime = -1;
    for (auto& rx : rxSignals) {
        if (rx.rxEndTime > maxRxEndTime)
            maxRxEndTime = rx.rxEndTime;
    }
    return maxRxEndTime;
}

void EthernetCsmaPhy::scheduleRxEndTimer()
{
    simtime_t rxEndTime = getMaxRxEndTime();
    EV_DEBUG << "Scheduling RX end timer" << EV_FIELD(rxEndTime) << EV_ENDL;
    if (!rxEndTimer->isScheduled())
        scheduleAt(rxEndTime, rxEndTimer);
    else if (rxEndTimer->getArrivalTime() != rxEndTime)
        rescheduleAt(rxEndTime, rxEndTimer);
}

void EthernetCsmaPhy::scheduleCrsOffTimer()
{
    simtime_t crsOffTime = std::max(getMaxRxEndTime(), txEndTime);
    EV_DEBUG << "Scheduling CRS off timer" << EV_FIELD(crsOffTime) << EV_ENDL;
    if (crsOffTime < simTime())
        cancelEvent(crsOffTimer);
    else if (!crsOffTimer->isScheduled())
        scheduleAt(crsOffTime, crsOffTimer);
    else if (crsOffTimer->getArrivalTime() != crsOffTime)
        rescheduleAt(crsOffTime, crsOffTimer);
}

} // namespace physicallayer

} // namespace inet

