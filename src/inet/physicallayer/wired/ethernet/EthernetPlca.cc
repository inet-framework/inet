//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPlca.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
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

EthernetPlca::~EthernetPlca()
{
    cancelAndDelete(beacon_timer);
    cancelAndDelete(beacon_det_timer);
    cancelAndDelete(burst_timer);
    cancelAndDelete(to_timer);
    cancelAndDelete(hold_timer);
    cancelAndDelete(pending_timer);
    cancelAndDelete(commit_timer);
    cancelAndDelete(tx_timer);
    delete currentTx;
    currentTx = nullptr;
}

void EthernetPlca::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        plca_node_count = par("plca_node_count");
        local_nodeID = par("local_nodeID");
        max_bc = par("max_bc");
        delay_line_length = par("delay_line_length");
        to_interval = par("to_interval");
        burst_interval = par("burst_interval");
        phy = getConnectedModule<IEthernetCsmaPhy>(gate("lowerLayerOut"));
        mac = getConnectedModule<IEthernetCsmaMac>(gate("upperLayerOut"));
        beacon_timer = new cMessage("beacon_timer", END_BEACON_TIMER);
        beacon_det_timer = new cMessage("beacon_det_timer", END_BEACON_DET_TIMER);
        burst_timer = new cMessage("burst_timer", END_BURST_TIMER);
        to_timer = new cMessage("to_timer", END_TO_TIMER);
        hold_timer = new cMessage("hold_timer", END_HOLD_TIMER);
        pending_timer = new cMessage("pending_timer", END_PENDING_TIMER);
        commit_timer = new cMessage("commit_timer", END_COMMIT_TIMER);
        tx_timer = new cMessage("tx_timer", END_TX_TIMER);
        emit(carrierSenseChangedSignal, (int)CRS);
        emit(collisionChangedSignal, (int)COL);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        auto networkInterface = getContainingNicModule(this);
        // TODO register to networkInterface parameter change signal and process changes
        mode = &EthernetModes::getEthernetMode(networkInterface->getDatarate());
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        controlFsm.setStateChangedSignal(controlStateChangedSignal);
        dataFsm.setStateChangedSignal(dataStateChangedSignal);
        controlFsm.setState(CS_DISABLE, "CS_DISABLE");
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
    emit(rxCmdSignal, getCmdCode(rx_cmd));
    emit(txCmdSignal, getCmdCode(tx_cmd));
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
        rx_cmd = "BEACON";
    }
    else if (signalType == COMMIT) {
        RX_DV = false;
        rx_cmd = "COMMIT";
    }
    else {
        RX_DV = true;
        rx_cmd = "NONE";
    }
    emit(rxCmdSignal, getCmdCode(rx_cmd));
    receiving = RX_DV || rx_cmd == "COMMIT";
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
    ASSERT(RX_DV || rx_cmd != "NONE");
    if (packet != nullptr)
        take(packet);
    EV_DEBUG << "Handling reception end" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    RX_DV = false;
    rx_cmd = "NONE";
    emit(rxCmdSignal, getCmdCode(rx_cmd));
    receiving = false;
    if (packet != nullptr)
        mac->handleReceptionEnd(signalType, packet, esd1);
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
                if (tx_cmd != "NONE") {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
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
                scheduleAfter(20 / mode->bitrate, beacon_timer);
                tx_cmd = "BEACON";
                emit(txCmdSignal, getCmdCode(tx_cmd));
                FSMA_Delay_Action(phy->startSignalTransmission(BEACON));
            );
            FSMA_Transition(T1,
                            !beacon_timer->isScheduled(),
                            CS_SYNCING,
            );
        }
        FSMA_State(CS_SYNCING) {
            FSMA_Enter(
                curID = 0;
                emit(curIDSignal, curID);
                if (tx_cmd != "NONE") {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
            );
            FSMA_Transition(T1,
                            !CRS,
                            CS_WAIT_TO,
            );
        }
        FSMA_State(CS_WAIT_TO) {
            FSMA_Enter(
                scheduleAfter(to_interval, to_timer);
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
                scheduleAfter(22 / mode->bitrate, beacon_det_timer);
            );
            // TODO this transition takes normal nodes to the first transmit opportunity too early because BEACON is detected at RECEPTION_START
            FSMA_Transition(T1, // D
                            local_nodeID != 0 && !receiving && (rx_cmd == "BEACON" || (!CRS && beacon_det_timer->isScheduled())),
                            CS_SYNCING,
            );
            FSMA_Transition(T2, // B
                            !CRS && local_nodeID != 0 && rx_cmd != "BEACON" && !beacon_det_timer->isScheduled(),
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
                tx_cmd = "COMMIT";
                emit(txCmdSignal, getCmdCode(tx_cmd));
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
                if (tx_cmd != "NONE") {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
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
                tx_cmd = "COMMIT";
                emit(txCmdSignal, getCmdCode(tx_cmd));
                FSMA_Delay_Action(phy->startSignalTransmission(COMMIT));
                scheduleAfter(burst_interval, burst_timer);
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
                if (tx_cmd != "NONE") {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESD));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
            );
            FSMA_Transition(T1,
                            !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
        }
        FSMA_State(CS_NEXT_TX_OPPORTUNITY) {
            FSMA_Enter(
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
                CARRIER_STATUS = "CARRIER_OFF";
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
                TX_EN = false;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(T1,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_TRANSMIT,
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Event_Transition(T2,
                                  event == CARRIER_SENSE_END,
                                  DS_IDLE,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_IDLE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_OFF";
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
                TX_EN = false;
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(T1,
                                  event == RECEPTION_START,
                                  DS_RECEIVE,
            );
            FSMA_Event_Transition(T2,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_HOLD,
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_RECEIVE) {
            FSMA_Enter(
                CARRIER_STATUS = CRS && rx_cmd != "COMMIT" ? "CARRIER_ON" : "CARRIER_OFF";
            );
            FSMA_Event_Transition(T1,
                                  event == RECEPTION_END,
                                  DS_IDLE,
                CARRIER_STATUS = CRS && rx_cmd != "COMMIT" ? "CARRIER_ON" : "CARRIER_OFF";
            );
            FSMA_Event_Transition(T2,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_COLLIDE,
                delete message;
            );
            FSMA_Event_Transition(T3,
                                  event == CARRIER_SENSE_START || event == CARRIER_SENSE_END,
                                  DS_RECEIVE,
                CARRIER_STATUS = CRS && rx_cmd != "COMMIT" ? "CARRIER_ON" : "CARRIER_OFF";
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_HOLD) {
            FSMA_Enter(
                packetPending = true;
                CARRIER_STATUS = "CARRIER_ON";
                scheduleAfter(delay_line_length * 4 / mode->bitrate, hold_timer);
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(T1,
                                  event == END_HOLD_TIMER,
                                  DS_COLLIDE,
                delete currentTx;
                currentTx = nullptr;
            );
            FSMA_Event_Transition(T2,
                                  event == RECEPTION_START,
                                  DS_COLLIDE,
                delete currentTx;
                currentTx = nullptr;
                cancelEvent(hold_timer);
            );
            FSMA_Event_Transition(T3,
                                  event == COMMIT_TO,
                                  DS_TRANSMIT,
                cancelEvent(hold_timer);
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_COLLIDE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_ON";
                SIGNAL_STATUS = "SIGNAL_ERROR";
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(T1,
                                  event == END_SIGNAL_TRANSMISSION,
                                  DS_DELAY_PENDING,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_DELAY_PENDING) {
            FSMA_Enter(
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
                scheduleAfter(512 / mode->bitrate, pending_timer)
            );
            FSMA_Event_Transition(T1,
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
            FSMA_Event_Transition(T1,
                                  event == COMMIT_TO,
                                  DS_WAIT_MAC,
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_START || event == CARRIER_SENSE_END);
            FSMA_Ignore_Event(event == RECEPTION_START || event == RECEPTION_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(DS_WAIT_MAC) {
            FSMA_Enter(
                CARRIER_STATUS = "CARRIER_OFF";
                scheduleAfter(288 / mode->bitrate, commit_timer)
            );
            FSMA_Event_Transition(T1,
                                  event == START_FRAME_TRANSMISSION,
                                  DS_TRANSMIT,
                cancelEvent(commit_timer);
                ASSERT(currentTx == nullptr);
                currentTx = check_and_cast<Packet *>(message);
            );
            FSMA_Event_Transition(T2,
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
                CARRIER_STATUS = "CARRIER_ON";
                SIGNAL_STATUS = COL ? "SIGNAL_ERROR" : "NO_SIGNAL_ERROR";
                TX_EN = true;
                if (tx_cmd != "NONE") { // KLUDGE: end commit signal
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
                simtime_t duration = b(currentTx->getDataLength() + ETHERNET_PHY_HEADER_LEN + getEsdLength()).get() / mode->bitrate;
                scheduleAfter(duration, tx_timer);
                ASSERT(macStartFrameTransmissionTime != -1);
                emit(packetPendingDelaySignal, simTime() - macStartFrameTransmissionTime);
                macStartFrameTransmissionTime = -1;
                if (phyStartFrameTransmissionTime != -1)
                    emit(packetIntervalSignal, simTime() - phyStartFrameTransmissionTime);
                phyStartFrameTransmissionTime = simTime();
                FSMA_Delay_Action(phy->startFrameTransmission(currentTx->dup(), bc < max_bc - 1 ? ESDBRS : ESD));
                FSMA_Delay_Action(handleWithControlFSM());
            );
            FSMA_Event_Transition(T1,
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
    if (CARRIER_STATUS == "CARRIER_OFF")
        new_carrier_sense_signal = false;
    else if (CARRIER_STATUS == "CARRIER_ON")
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
    if (SIGNAL_STATUS == "NO_SIGNAL_ERROR")
        new_collision_signal = false;
    else if (SIGNAL_STATUS == "SIGNAL_ERROR")
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

int EthernetPlca::getCmdCode(std::string cmd)
{
    if (cmd == "NONE")
        return 0;
    else if (cmd == "BEACON")
        return 1;
    else if (cmd == "COMMIT")
        return 2;
    else
        throw cRuntimeError("Unknown cmd");
}

} // namespace physicallayer

} // namespace inet

