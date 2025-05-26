//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPlca.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/PacketEventTag.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/TimeTag.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPlca);

simsignal_t EthernetPlca::curIDSignal = cComponent::registerSignal("curID");
simsignal_t EthernetPlca::carrierSenseChangedSignal = cComponent::registerSignal("carrierSenseChanged");
simsignal_t EthernetPlca::collisionChangedSignal = cComponent::registerSignal("collisionChanged");
simsignal_t EthernetPlca::controlStateChangedSignal = cComponent::registerSignal("controlStateChanged");
simsignal_t EthernetPlca::dataStateChangedSignal = cComponent::registerSignal("dataStateChanged");
simsignal_t EthernetPlca::rxCmdSignal = cComponent::registerSignal("rxCmd");
simsignal_t EthernetPlca::txCmdSignal = cComponent::registerSignal("txCmd");
simsignal_t EthernetPlca::packetPendingDelaySignal = cComponent::registerSignal("packetPendingDelay");
simsignal_t EthernetPlca::packetIntervalSignal = cComponent::registerSignal("packetInterval");
simsignal_t EthernetPlca::transmitOpportunityUsedSignal = cComponent::registerSignal("transmitOpportunityUsed");
simsignal_t EthernetPlca::numPacketsPerToSignal = cComponent::registerSignal("numPacketsPerTo");
simsignal_t EthernetPlca::numPacketsPerOwnToSignal = cComponent::registerSignal("numPacketsPerOwnTo");
simsignal_t EthernetPlca::numPacketsPerCycleSignal = cComponent::registerSignal("numPacketsPerCycle");
simsignal_t EthernetPlca::toLengthSignal = cComponent::registerSignal("toLength");
simsignal_t EthernetPlca::ownToLengthSignal = cComponent::registerSignal("ownToLength");
simsignal_t EthernetPlca::cycleLengthSignal = cComponent::registerSignal("cycleLength");

Register_Enum(EthernetPlca::ControlState,
    (EthernetPlca::CS_DISABLE,
     EthernetPlca::CS_RESYNC,
     EthernetPlca::CS_RECOVER,
     EthernetPlca::CS_SEND_BEACON,
     EthernetPlca::CS_SYNCING,
     EthernetPlca::CS_WAIT_TO,
     EthernetPlca::CS_EARLY_RECEIVE,
     EthernetPlca::CS_COMMIT,
     EthernetPlca::CS_YIELD,
     EthernetPlca::CS_RECEIVE,
     EthernetPlca::CS_TRANSMIT,
     EthernetPlca::CS_BURST,
     EthernetPlca::CS_ABORT,
     EthernetPlca::CS_NEXT_TX_OPPORTUNITY));

Register_Enum(EthernetPlca::DataState,
    (EthernetPlca::DS_WAIT_IDLE,
     EthernetPlca::DS_IDLE,
     EthernetPlca::DS_RECEIVE,
     EthernetPlca::DS_HOLD,
     EthernetPlca::DS_COLLIDE,
     EthernetPlca::DS_DELAY_PENDING,
     EthernetPlca::DS_PENDING,
     EthernetPlca::DS_WAIT_MAC,
     EthernetPlca::DS_TRANSMIT));

Register_Enum(EthernetPlca::CARRIER_STATUS_ENUM,
    (EthernetPlca::CARRIER_OFF,
     EthernetPlca::CARRIER_ON));
Register_Enum(EthernetPlca::SIGNAL_STATUS_ENUM,
    (EthernetPlca::NO_SIGNAL_ERROR,
     EthernetPlca::SIGNAL_ERROR));
Register_Enum(EthernetPlca::CMD_ENUM,
    (EthernetPlca::CMD_NONE,
     EthernetPlca::CMD_BEACON,
     EthernetPlca::CMD_COMMIT));

EthernetPlca::~EthernetPlca()
{
    cancelAndDelete(beacon_timer);
    cancelAndDelete(beacon_det_timer);
    cancelAndDelete(burst_timer);
    cancelAndDelete(to_timer);
    cancelAndDelete(syncing_timer);
    cancelAndDelete(hold_timer);
    cancelAndDelete(pending_timer);
    cancelAndDelete(commit_timer);
    cancelAndDelete(tx_timer);
    delete currentTx;
    currentTx = nullptr;
}

void EthernetPlca::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        plca_node_count = par("plca_node_count");
        local_nodeID = par("local_nodeID");
        max_bc = par("max_bc");
        delay_line_length = par("delay_line_length");
        to_timer_length = b(par("to_timer_length"));
        burst_timer_length = b(par("burst_timer_length"));
        beacon_timer_length = b(par("beacon_timer_length"));
        beacon_det_timer_length = b(par("beacon_det_timer_length"));
        pending_timer_length = b(par("pending_timer_length"));
        commit_timer_length = b(par("commit_timer_length"));
        phy = getConnectedModule<IEthernetCsmaPhy>(gate("lowerLayerOut"));
        mac = getConnectedModule<IEthernetCsmaMac>(gate("upperLayerOut"));
        beacon_timer = new cMessage("beacon_timer", END_BEACON_TIMER);
        beacon_det_timer = new cMessage("beacon_det_timer", END_BEACON_DET_TIMER);
        burst_timer = new cMessage("burst_timer", END_BURST_TIMER);
        to_timer = new cMessage("to_timer", END_TO_TIMER);
        // schedule so that the control state machine doesn't yield when a packet arrives at the same time as the transmit opportunity start
        to_timer->setSchedulingPriority(100);
        syncing_timer = new cMessage("syncing_timer", END_SYNCING_TIMER);
        hold_timer = new cMessage("hold_timer", END_HOLD_TIMER);
        pending_timer = new cMessage("pending_timer", END_PENDING_TIMER);
        commit_timer = new cMessage("commit_timer", END_COMMIT_TIMER);
        tx_timer = new cMessage("tx_timer", END_TX_TIMER);
        curID = plca_node_count;
        emit(curIDSignal, curID);
        emit(carrierSenseChangedSignal, (int)CRS);
        emit(collisionChangedSignal, (int)COL);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        auto networkInterface = getContainingNicModule(this);
        // TODO register to networkInterface parameter change signal and process changes
        mode = EthernetModes::getEthernetMode(networkInterface->getDatarate());
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        controlFsm.setStateChangedSignal(controlStateChangedSignal);
        dataFsm.setStateChangedSignal(dataStateChangedSignal);
        controlFsm.setState(CS_RESYNC, "CS_RESYNC");
        dataFsm.setState(DS_IDLE, "DS_IDLE");
        handleWithControlFSM();
    }
}

void EthernetPlca::finish()
{
    emit(curIDSignal, curID);
    emit(carrierSenseChangedSignal, (int)CRS);
    emit(collisionChangedSignal, (int)COL);
    emit(controlStateChangedSignal, controlFsm.getState());
    emit(dataStateChangedSignal, dataFsm.getState());
    emit(rxCmdSignal, rx_cmd);
    emit(txCmdSignal, tx_cmd);
}

void EthernetPlca::refreshDisplay() const
{
    auto& displayString = getDisplayString();
    std::stringstream stream;
    stream << curID << "/" << plca_node_count << " (" << local_nodeID << ")\n";
    stream << controlFsm.getStateName() << " - " << dataFsm.getStateName();
    displayString.setTagArg("t", 0, stream.str().c_str());
}

void EthernetPlca::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else
        throw cRuntimeError("Unknown message");
}

void EthernetPlca::handleSelfMessage(cMessage *message)
{
    EV_DEBUG << "Handling self message" << EV_FIELD(message) << EV_ENDL;
    int kind = message->getKind();
    handleWithControlFSM();
    if (kind == END_HOLD_TIMER || kind == END_PENDING_TIMER || kind == END_COMMIT_TIMER || kind == END_TX_TIMER)
        handleWithDataFSM(kind, message);
}

void EthernetPlca::handleCarrierSenseStart()
{
    Enter_Method("handleCarrierSenseStart");
    ASSERT(!CRS);
    EV_DEBUG << "Handling carrier sense start" << EV_ENDL;
    CRS = true;
    emit(carrierSenseChangedSignal, (int)CRS);
    handleWithControlFSM();
    handleWithDataFSM(CARRIER_SENSE_START, nullptr);
}

void EthernetPlca::handleCarrierSenseEnd()
{
    Enter_Method("handleCarrierSenseEnd");
    ASSERT(CRS);
    EV_DEBUG << "Handling carrier sense end" << EV_ENDL;
    CRS = false;
    emit(carrierSenseChangedSignal, (int)CRS);
    handleWithControlFSM();
    handleWithDataFSM(CARRIER_SENSE_END, nullptr);
}

void EthernetPlca::handleCollisionStart()
{
    Enter_Method("handleCollisionStart");
    ASSERT(!COL);
    EV_DEBUG << "Handling collision start" << EV_ENDL;
    COL = true;
    emit(collisionChangedSignal, (int)COL);
    handleWithDataFSM(COLLISION_START, nullptr);
}

void EthernetPlca::handleCollisionEnd()
{
    Enter_Method("handleCollisionEnd");
    ASSERT(COL);
    EV_DEBUG << "Handling collision end" << EV_ENDL;
    COL = false;
    emit(collisionChangedSignal, (int)COL);
    handleWithDataFSM(COLLISION_END, nullptr);
}

void EthernetPlca::handleReceptionStart(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionStart");
    ASSERT(!RX_DV);
    if (packet != nullptr)
        take(packet);
    EV_DEBUG << "Handling reception start" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (signalType == BEACON) {
        RX_DV = false;
        rx_cmd = CMD_BEACON;
    }
    else if (signalType == COMMIT) {
        RX_DV = false;
        rx_cmd = CMD_COMMIT;
    }
    else {
        RX_DV = true;
        rx_cmd = CMD_NONE;
    }
    emit(rxCmdSignal, rx_cmd);
    receiving = RX_DV || rx_cmd == CMD_COMMIT;
    handleWithDataFSM(RECEPTION_START, packet);
    if (packet != nullptr)
        mac->handleReceptionStart(signalType, packet);
}

void EthernetPlca::handleReceptionError(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionError");
    if (packet != nullptr)
        take(packet);
    delete packet;
    throw cRuntimeError("TODO");
}

void EthernetPlca::handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("handleReceptionEnd");
    ASSERT(RX_DV || rx_cmd != CMD_NONE);
    if (packet != nullptr)
        take(packet);
    EV_DEBUG << "Handling reception end" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    RX_DV = false;
    rx_cmd = CMD_NONE;
    emit(rxCmdSignal, rx_cmd);
    receiving = false;
    if (packet != nullptr) {
        numPacketsPerTo++;
        numPacketsPerCycle++;
        emit(packetReceivedFromLowerSignal, packet);
        mac->handleReceptionEnd(signalType, packet, esd1);
    }
    handleWithDataFSM(RECEPTION_END, packet);
}

void EthernetPlca::startSignalTransmission(EthernetSignalType signalType)
{
    Enter_Method("startSignalTransmission");
    EV_DEBUG << "Starting signal transmission" << EV_FIELD(signalType) << EV_ENDL;
    if (signalType == JAM) {
        // JAM signals are not sent to the PHY
    }
    else
        throw cRuntimeError("Invalid operation");
}

void EthernetPlca::endSignalTransmission(EthernetEsdType esd1)
{
    Enter_Method("endSignalTransmission");
    EV_DEBUG << "Ending signal transmission" << EV_ENDL;
    handleWithDataFSM(END_SIGNAL_TRANSMISSION, nullptr);
}

void EthernetPlca::startFrameTransmission(Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("startFrameTransmission");
    EV_DEBUG << "Starting frame transmission" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    if (macStartFrameTransmissionTime == -1)
        macStartFrameTransmissionTime = simTime();
    handleWithDataFSM(START_FRAME_TRANSMISSION, packet);
}

void EthernetPlca::endFrameTransmission()
{
    Enter_Method("endFrameTransmission");
    EV_DEBUG << "Ending frame transmission" << EV_ENDL;
}

void EthernetPlca::handleWithControlFSM()
{
    // 148.4.4.6 State diagram of IEEE Std 802.3cg-2019
    { FSMA_Switch(controlFsm) {
        FSMA_State(CS_DISABLE) {
            FSMA_Enter(
                if (tx_cmd != CMD_NONE) {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = CMD_NONE;
                    emit(txCmdSignal, tx_cmd);
                }
                committed = false;
                curID = 0;
                emit(curIDSignal, curID);
            );
            FSMA_Transition(T1,
                            local_nodeID != 0,
                            CS_RESYNC,
            );
            FSMA_Transition(T2,
                            local_nodeID == 0,
                            CS_RECOVER,
            );
        }
        FSMA_State(CS_RESYNC) {
            FSMA_Enter(
            );
            FSMA_Transition(T1,
                            local_nodeID != 0 && CRS,
                            CS_EARLY_RECEIVE,
            );
            FSMA_Transition(T2,
                            PMCD && !CRS && local_nodeID == 0,
                            CS_SEND_BEACON,
            );
        }
        FSMA_State(CS_RECOVER) {
            FSMA_Enter(
            );
            FSMA_Transition(T1,
                            true,
                            CS_WAIT_TO,
            );
        }
        FSMA_State(CS_SEND_BEACON) {
            FSMA_Enter(
                scheduleAfter(beacon_timer_length.get<b>() / mode.bitrate, beacon_timer);
                tx_cmd = CMD_BEACON;
                emit(txCmdSignal, tx_cmd);
                FSMA_Delay_Action(phy->startSignalTransmission(BEACON));
            );
            FSMA_Transition(T1,
                            !beacon_timer->isScheduled(),
                            CS_SYNCING,
            );
        }
        FSMA_State(CS_SYNCING) {
            FSMA_Enter(
                // curID = 0; NOTE: this is removed so that the curID gets zero exactly at the first TO start in both the contoller and other nodes
                // emit(curIDSignal, curID);
                if (tx_cmd != CMD_NONE) {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = CMD_NONE;
                    emit(txCmdSignal, tx_cmd);
                }
                if (local_nodeID == 0)
                    // the syncing timer separates the BEACON and the following COMMIT/DATA signals with a small amount of time
                    // this is done for non-controller nodes to have a CRS OFF/ON spike in the SYNCING state to allow them to go into WAIT_TO
                    scheduleAfter(1E-9, syncing_timer);
            );
            FSMA_Transition(T1,
                            !CRS && !syncing_timer->isScheduled(),
                            CS_WAIT_TO,
                curID = 0;
                emit(curIDSignal, curID);
                if (cycleStartTime != -1) {
                    emit(numPacketsPerCycleSignal, numPacketsPerCycle);
                    emit(cycleLengthSignal, simTime() - cycleStartTime);
                }
                numPacketsPerCycle = 0;
                cycleStartTime = simTime();
            );
        }
        FSMA_State(CS_WAIT_TO) {
            FSMA_Enter(
                scheduleAfter(to_timer_length.get<b>() / mode.bitrate, to_timer);
                numPacketsPerTo = 0;
                toStartTime = simTime();
            );
            FSMA_Transition(T1,
                            CRS,
                            CS_EARLY_RECEIVE,
            );
            FSMA_Transition(T2,
                            curID == local_nodeID && packetPending && !CRS,
                            CS_COMMIT,
            );
            FSMA_Transition(T3,
                            !to_timer->isScheduled() && curID != local_nodeID && !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T4,
                            curID == local_nodeID && !packetPending && !CRS,
                            CS_YIELD,
            );
        }
        FSMA_State(CS_EARLY_RECEIVE) {
            FSMA_Enter(
                cancelEvent(to_timer);
                rescheduleAfter(beacon_det_timer_length.get<b>() / mode.bitrate, beacon_det_timer);
            );
            // TODO this transition takes normal nodes to the first transmit opportunity too early because BEACON is detected at RECEPTION_START
            FSMA_Transition(T1, // D
                            local_nodeID != 0 && !receiving && (rx_cmd == CMD_BEACON || (!CRS && beacon_det_timer->isScheduled())),
                            CS_SYNCING,
            );
            FSMA_Transition(T2, // B
                            !CRS && local_nodeID != 0 && rx_cmd != CMD_BEACON && !beacon_det_timer->isScheduled(),
                            CS_RESYNC,
            );
            FSMA_Transition(T3, // C
                            !CRS && local_nodeID == 0,
                            CS_RECOVER,
            );
            FSMA_Transition(T4,
                            receiving && CRS,
                            CS_RECEIVE,
            );
        }
        FSMA_State(CS_COMMIT) {
            FSMA_Enter(
                tx_cmd = CMD_COMMIT;
                emit(txCmdSignal, tx_cmd);
                FSMA_Delay_Action(phy->startSignalTransmission(COMMIT));
                committed = true;
                cancelEvent(to_timer);
                bc = 0;
                emit(transmitOpportunityUsedSignal, 1);
                FSMA_Delay_Action(handleWithDataFSM(COMMIT_TO, nullptr));
            );
            FSMA_Transition(T1,
                            TX_EN,
                            CS_TRANSMIT,
            );
            FSMA_Transition(T2,
                            !TX_EN && !packetPending,
                            CS_ABORT,
            );
        }
        FSMA_State(CS_YIELD) {
            FSMA_Enter(
                emit(transmitOpportunityUsedSignal, 0);
            );
            FSMA_Transition(T1,
                            CRS && to_timer->isScheduled(),
                            CS_EARLY_RECEIVE,
            );
            FSMA_Transition(T2,
                            !to_timer->isScheduled(),
                            CS_NEXT_TX_OPPORTUNITY,
            );
        }
        FSMA_State(CS_RECEIVE) {
            FSMA_Transition(T1,
                            !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
        }
        FSMA_State(CS_TRANSMIT) {
            FSMA_Enter(
                if (tx_cmd != CMD_NONE) {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = CMD_NONE;
                    emit(txCmdSignal, tx_cmd);
                }
                if (bc >= max_bc)
                    committed = false;
            );
            FSMA_Transition(T1,
                            !TX_EN && !CRS && bc >= max_bc,
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T2,
                            !TX_EN && bc < max_bc,
                            CS_BURST,
            );
        }
        FSMA_State(CS_BURST) {
            FSMA_Enter(
                bc = bc + 1;
                tx_cmd = CMD_COMMIT;
                emit(txCmdSignal, tx_cmd);
                FSMA_Delay_Action(phy->startSignalTransmission(COMMIT));
                scheduleAfter(burst_timer_length.get<b>() / mode.bitrate, burst_timer);
            );
            FSMA_Transition(T1,
                            TX_EN,
                            CS_TRANSMIT,
                cancelEvent(burst_timer);
            );
            FSMA_Transition(T2,
                            !TX_EN && !burst_timer->isScheduled(),
                            CS_ABORT,
            );
        }
        FSMA_State(CS_ABORT) {
            FSMA_Enter(
                if (tx_cmd != CMD_NONE) {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESD));
                    tx_cmd = CMD_NONE;
                    emit(txCmdSignal, tx_cmd);
                }
            );
            FSMA_Transition(T1,
                            !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
        }
        FSMA_State(CS_NEXT_TX_OPPORTUNITY) {
            FSMA_Enter(
                if (toStartTime != -1) {
                    emit(numPacketsPerToSignal, numPacketsPerTo);
                    emit(toLengthSignal, simTime() - toStartTime);
                    if (curID == local_nodeID) {
                        emit(numPacketsPerOwnToSignal, numPacketsPerTo);
                        emit(ownToLengthSignal, simTime() - toStartTime);
                    }
                    numPacketsPerTo = -1;
                    toStartTime = -1;
                }
                curID = curID + 1;
                emit(curIDSignal, curID);
                committed = false;
            );
            FSMA_Transition(T1, // B
                            (local_nodeID == 0 && curID >= plca_node_count),
                            CS_RESYNC,
            );
            FSMA_Transition(T2,
                            true,
                            CS_WAIT_TO,
            );
        }
    } }
    controlFsm.executeDelayedActions();
}

void EthernetPlca::handleWithDataFSM(int event, cMessage *message)
{
    // 148.4.5.7 State diagram of IEEE Std 802.3cg-2019
    // Modified state machine for discrete event simulation
    { FSMA_Switch(dataFsm) {
        FSMA_State(DS_WAIT_IDLE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = CARRIER_OFF;
                SIGNAL_STATUS = NO_SIGNAL_ERROR;
                TX_EN = false;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(TX_START,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_TRANSMIT,
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Event_Transition(CRS_END,
                                  event == CARRIER_SENSE_END,
                                  DS_IDLE,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_IDLE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = CARRIER_OFF;
                SIGNAL_STATUS = NO_SIGNAL_ERROR;
                TX_EN = false;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(RX_START,
                                  event == RECEPTION_START && receiving,
                                  DS_RECEIVE,
            );
            FSMA_Event_Transition(TX_START,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_HOLD,
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event((event == RECEPTION_START && !receiving) || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_RECEIVE) {
            FSMA_Enter(
                CARRIER_STATUS = CRS && rx_cmd != CMD_COMMIT ? CARRIER_ON : CARRIER_OFF;
            );
            FSMA_Event_Transition(RX_END,
                                  event == RECEPTION_END,
                                  DS_IDLE,
                CARRIER_STATUS = CRS && rx_cmd != CMD_COMMIT ? CARRIER_ON : CARRIER_OFF;
            );
            FSMA_Event_Transition(TX_START,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_COLLIDE,
                delete message;
            );
            FSMA_Event_Transition(CRS_CHANGE,
                                  event == CARRIER_SENSE_START || event == CARRIER_SENSE_END,
                                  DS_RECEIVE,
                CARRIER_STATUS = CRS && rx_cmd != CMD_COMMIT ? CARRIER_ON : CARRIER_OFF;    // TODO is this required? FSMA_Enter() is the same...
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_HOLD) {
            FSMA_Enter(
                packetPending = true;
                CARRIER_STATUS = CARRIER_ON;
                scheduleAfter(delay_line_length * 4 / mode.bitrate, hold_timer);
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(HOLD_END,
                                  event == END_HOLD_TIMER,
                                  DS_COLLIDE,
                delete currentTx;
                currentTx = nullptr;
            );
            FSMA_Event_Transition(RX_START,
                                  event == RECEPTION_START,
                                  DS_COLLIDE,
                delete currentTx;
                currentTx = nullptr;
                cancelEvent(hold_timer);
            );
            FSMA_Event_Transition(COMMIT_TO,
                                  event == COMMIT_TO,
                                  DS_TRANSMIT,
                cancelEvent(hold_timer);
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_END); // beacon
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_COLLIDE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = CARRIER_ON;
                SIGNAL_STATUS = SIGNAL_ERROR;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(JAM_END,
                                  event == END_SIGNAL_TRANSMISSION, // TODO check for JAM signal type
                                  DS_DELAY_PENDING,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_DELAY_PENDING) {
            FSMA_Enter(
                SIGNAL_STATUS = NO_SIGNAL_ERROR;
                scheduleAfter(pending_timer_length.get<b>() / mode.bitrate, pending_timer)
            );
            FSMA_Event_Transition(PENDING_END,
                                  event == END_PENDING_TIMER,
                                  DS_PENDING,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_PENDING) {
            FSMA_Enter(
                packetPending = true;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(COMMIT_TO,
                                  event == COMMIT_TO,
                                  DS_WAIT_MAC,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_WAIT_MAC) {
            FSMA_Enter(
                CARRIER_STATUS = CARRIER_OFF;
                scheduleAfter(commit_timer_length.get<b>() / mode.bitrate, commit_timer)
            );
            FSMA_Event_Transition(TX_START,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_TRANSMIT,
                cancelEvent(commit_timer);
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Event_Transition(COMMIT_END,
                                  event == END_COMMIT_TIMER,
                                  DS_WAIT_IDLE,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_TRANSMIT) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = CARRIER_ON;
                SIGNAL_STATUS = COL ? SIGNAL_ERROR : NO_SIGNAL_ERROR;
                TX_EN = true;
                if (tx_cmd != CMD_NONE) { // KLUDGE: end commit signal
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = CMD_NONE;
                    emit(txCmdSignal, tx_cmd);
                }
                simtime_t duration = (currentTx->getDataLength() + ETHERNET_PHY_HEADER_LEN + getEsdLength()).get<b>() / mode.bitrate;
                scheduleAfter(duration, tx_timer);
                ASSERT(macStartFrameTransmissionTime != -1);
                simtime_t packetPendingDelay = simTime() - macStartFrameTransmissionTime;
                emit(packetPendingDelaySignal, packetPendingDelay);
                auto packet = currentTx->dup();
                insertPacketEvent(this, packet, PEK_PROCESSED, 0, packetPendingDelay);
                increaseTimeTag<PropagationTimeTag>(packet, packetPendingDelay, packetPendingDelay);
                macStartFrameTransmissionTime = -1;
                if (phyStartFrameTransmissionTime != -1)
                    emit(packetIntervalSignal, simTime() - phyStartFrameTransmissionTime);
                phyStartFrameTransmissionTime = simTime();
                emit(packetSentToLowerSignal, packet);
                numPacketsPerTo++;
                numPacketsPerCycle++;
                FSMA_Delay_Action(phy->startFrameTransmission(packet, bc < max_bc - 1 ? ESDBRS : ESD));
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(TX_END,
                                  event == END_TX_TIMER,
                                  DS_WAIT_IDLE,
                delete currentTx;
                currentTx = nullptr;
                FSMA_Delay_Action(phy->endFrameTransmission());
            );
            // TODO this transition would execute FSMA_Enter again
//            FSMA_Event_Transition(T2,
//                                  event == COLLISION_START,
//                                  DS_TRANSMIT,
//                SIGNAL_STATUS = "SIGNAL_ERROR";
//            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
    } }
    dataFsm.executeDelayedActions();
    // call handleCarrierSenseStart/handleCarrierSenseEnd if needed
    bool new_carrier_sense_signal;
    if (CARRIER_STATUS == CARRIER_OFF)
        new_carrier_sense_signal = false;
    else if (CARRIER_STATUS == CARRIER_ON)
        new_carrier_sense_signal = true;
    else
        throw cRuntimeError("Unknown carrier status");
    if (new_carrier_sense_signal != old_carrier_sense_signal) {
        old_carrier_sense_signal = new_carrier_sense_signal;
        if (new_carrier_sense_signal)
            mac->handleCarrierSenseStart();
        else
            mac->handleCarrierSenseEnd();
    }
    // call handleCollisionStart/handleCollisionEnd if needed
    bool new_collision_signal;
    if (SIGNAL_STATUS == NO_SIGNAL_ERROR)
        new_collision_signal = false;
    else if (SIGNAL_STATUS == SIGNAL_ERROR)
        new_collision_signal = true;
    else
        throw cRuntimeError("Unknown signal status");
    if (new_collision_signal != old_collision_signal) {
        old_collision_signal = new_collision_signal;
        if (new_collision_signal)
            mac->handleCollisionStart();
        else
            mac->handleCollisionEnd();
    }
}

} // namespace physicallayer

} // namespace inet

