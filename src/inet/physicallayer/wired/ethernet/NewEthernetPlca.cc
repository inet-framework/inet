//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/NewEthernetPlca.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(NewEthernetPlca);

simsignal_t NewEthernetPlca::curIDSignal = cComponent::registerSignal("curID");
simsignal_t NewEthernetPlca::controlStateChangedSignal = cComponent::registerSignal("controlStateChanged");
simsignal_t NewEthernetPlca::dataStateChangedSignal = cComponent::registerSignal("dataStateChanged");
simsignal_t NewEthernetPlca::statusStateChangedSignal = cComponent::registerSignal("statusStateChanged");
simsignal_t NewEthernetPlca::rxCmdSignal = cComponent::registerSignal("rxCmd");
simsignal_t NewEthernetPlca::txCmdSignal = cComponent::registerSignal("txCmd");

Register_Enum(NewEthernetPlca::ControlState,
    (NewEthernetPlca::CS_DISABLE,
     NewEthernetPlca::CS_RESYNC,
     NewEthernetPlca::CS_RECOVER,
     NewEthernetPlca::CS_SEND_BEACON,
     NewEthernetPlca::CS_SYNCING,
     NewEthernetPlca::CS_WAIT_TO,
     NewEthernetPlca::CS_EARLY_RECEIVE,
     NewEthernetPlca::CS_COMMIT,
     NewEthernetPlca::CS_YIELD,
     NewEthernetPlca::CS_RECEIVE,
     NewEthernetPlca::CS_TRANSMIT,
     NewEthernetPlca::CS_BURST,
     NewEthernetPlca::CS_ABORT,
     NewEthernetPlca::CS_NEXT_TX_OPPORTUNITY));

Register_Enum(NewEthernetPlca::DataState,
    (NewEthernetPlca::DS_NORMAL,
     NewEthernetPlca::DS_WAIT_IDLE,
     NewEthernetPlca::DS_IDLE,
     NewEthernetPlca::DS_RECEIVE,
     NewEthernetPlca::DS_HOLD,
     NewEthernetPlca::DS_ABORT,
     NewEthernetPlca::DS_COLLIDE,
     NewEthernetPlca::DS_DELAY_PENDING,
     NewEthernetPlca::DS_PENDING,
     NewEthernetPlca::DS_WAIT_MAC,
     NewEthernetPlca::DS_TRANSMIT,
     NewEthernetPlca::DS_FLUSH));

Register_Enum(NewEthernetPlca::StatusState,
    (NewEthernetPlca::SS_INACTIVE,
     NewEthernetPlca::SS_ACTIVE,
     NewEthernetPlca::SS_HYSTERESIS));

NewEthernetPlca::~NewEthernetPlca()
{
    cancelAndDelete(beacon_timer);
    cancelAndDelete(beacon_det_timer);
    cancelAndDelete(invalid_beacon_timer);
    cancelAndDelete(burst_timer);
    cancelAndDelete(to_timer);
    cancelAndDelete(hold_timer);
    cancelAndDelete(pending_timer);
    cancelAndDelete(commit_timer);
    cancelAndDelete(plca_status_timer);
}

void NewEthernetPlca::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        plca_en = par("plca_en");
        plca_reset = par("plca_reset");
        plca_node_count = par("plca_node_count");
        local_nodeID = par("local_nodeID");
        max_bc = par("max_bc");
        delay_line_length = par("delay_line_length");
        to_interval = par("to_interval");
        burst_interval = par("burst_interval");
        phy = getConnectedModule<IEthernetCsmaPhy>(gate("lowerLayerOut"));
        mac = getConnectedModule<IEthernetCsmaMac>(gate("upperLayerOut"));
        beacon_timer = new cMessage("beacon_timer");
        beacon_det_timer = new cMessage("beacon_det_timer");
        invalid_beacon_timer = new cMessage("invalid_beacon_timer");
        burst_timer = new cMessage("burst_timer");
        to_timer = new cMessage("to_timer");
        hold_timer = new cMessage("hold_timer");
        pending_timer = new cMessage("pending_timer");
        commit_timer = new cMessage("commit_timer");
        plca_status_timer = new cMessage("plca_status_timer");
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        controlFsm.setStateChangedSignal(controlStateChangedSignal);
        dataFsm.setStateChangedSignal(dataStateChangedSignal);
        statusFsm.setStateChangedSignal(statusStateChangedSignal);
        controlFsm.setState(CS_DISABLE, "CS_DISABLE");
        dataFsm.setState(DS_NORMAL, "DS_NORMAL");
        statusFsm.setState(SS_INACTIVE, "SS_INACTIVE");
        handleWithFSMs();
    }
}

void NewEthernetPlca::handleParameterChange(const char *name)
{
    if (!strcmp(name, "plca_en")) {
        plca_en = par("plca_en");
        handleWithFSMs();
    }
    else if (!strcmp(name, "plca_reset")) {
        plca_reset = par("plca_reset");
        handleWithFSMs();
    }
    else
        throw cRuntimeError("Unknown parameter name");
}

void NewEthernetPlca::refreshDisplay() const
{
    auto& displayString = getDisplayString();
    std::stringstream stream;
    stream << curID << "/" << plca_node_count << " (" << local_nodeID << ")\n";
    stream << controlFsm.getStateName() << " - " << dataFsm.getStateName() << " - " << statusFsm.getStateName();
    displayString.setTagArg("t", 0, stream.str().c_str());
}

void NewEthernetPlca::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    // TODO remove, not need because state machine handles all events
    else if (message->getArrivalGate() == gate("upperLayerIn"))
        ; // send(message, "lowerLayerOut");
    // TODO remove, not need because state machine handles all events
    else if (message->getArrivalGate() == gate("lowerLayerIn"))
        ; // send(message, "upperLayerOut");
    else
        throw cRuntimeError("Unknown message");
}

void NewEthernetPlca::handleSelfMessage(cMessage *message)
{
    EV_DEBUG << "Handling self message" << EV_FIELD(message) << EV_ENDL;
    handleWithFSMs();
}

void NewEthernetPlca::handleCarrierSenseStart()
{
    Enter_Method("handleCarrierSenseStart");
    ASSERT(!CRS);
    EV_DEBUG << "Handling carrier sense start" << EV_ENDL;
    CRS = true;
    handleWithFSMs();
}

void NewEthernetPlca::handleCarrierSenseEnd()
{
    Enter_Method("handleCarrierSenseEnd");
    ASSERT(CRS);
    EV_DEBUG << "Handling carrier sense end" << EV_ENDL;
    CRS = false;
    handleWithFSMs();
}

void NewEthernetPlca::handleCollisionStart()
{
    Enter_Method("handleCollisionStart");
    ASSERT(!COL);
    EV_DEBUG << "Handling collision start" << EV_ENDL;
    COL = true;
    handleWithFSMs();
}

void NewEthernetPlca::handleCollisionEnd()
{
    Enter_Method("handleCollisionEnd");
    ASSERT(COL);
    EV_DEBUG << "Handling collision end" << EV_ENDL;
    COL = false;
    handleWithFSMs();
}

void NewEthernetPlca::handleReceptionStart(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionStart");
    ASSERT(!RX_DV);
    EV_DEBUG << "Handling reception start" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    RX_DV = true;
    if (signalType == BEACON)
        rx_cmd = "BEACON";
    else if (signalType == COMMIT)
        rx_cmd = "COMMIT";
    else
        rx_cmd = "NONE";
    emit(rxCmdSignal, getCmdCode(rx_cmd));
    receiving = RX_DV || rx_cmd == "COMMIT";
    handleWithFSMs();
    if (packet != nullptr)
        mac->handleReceptionStart(signalType, packet);
}

void NewEthernetPlca::handleReceptionError(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionError");
    throw cRuntimeError("TODO");
}

void NewEthernetPlca::handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("handleReceptionEnd");
    ASSERT(RX_DV);
    EV_DEBUG << "Handling reception end" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    RX_DV = false;
    rx_cmd = "NONE";
    emit(rxCmdSignal, getCmdCode(rx_cmd));
    receiving = false;
    handleWithFSMs();
    if (packet != nullptr)
        mac->handleReceptionEnd(signalType, packet, esd1);
}

void NewEthernetPlca::startSignalTransmission(EthernetSignalType signalType)
{
    Enter_Method("startSignalTransmission");
    EV_DEBUG << "Starting signal transmission" << EV_FIELD(signalType) << EV_ENDL;
    if (signalType == JAM) {
        plca_txen = true;
        plca_txd = nullptr;
        plca_txtype = JAM;
        handleWithFSMs();
    }
    else
        throw cRuntimeError("Invalid operation");
}

void NewEthernetPlca::endSignalTransmission(EthernetEsdType esd1)
{
    Enter_Method("endSignalTransmission");
    EV_DEBUG << "Ending signal transmission" << EV_ENDL;
    plca_txen = false;
    plca_txd = nullptr;
    plca_txtype = NONE;
    handleWithFSMs();
}

void NewEthernetPlca::startFrameTransmission(Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("startFrameTransmission");
    EV_DEBUG << "Starting frame transmission" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    plca_txen = true;
    plca_txd = packet;
    plca_txtype = DATA;
    handleWithFSMs();
}

void NewEthernetPlca::endFrameTransmission()
{
    Enter_Method("endFrameTransmission");
    EV_DEBUG << "Ending frame transmission" << EV_ENDL;
    plca_txen = false;
    plca_txd = nullptr;
    plca_txtype = NONE;
    handleWithFSMs();
}

void NewEthernetPlca::handleWithControlFSM()
{
    // 148.4.4.6 State diagram of IEEE Std 802.3cg-2019
    { FSMA_Switch(controlFsm) {
        FSMA_State(CS_DISABLE) {
            FSMA_Enter(
                if (tx_cmd != "NONE") {
//                    FSMA_Delay_Action(phy->endSignalTransmission());
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
                committed = false;
                curID = 0;
                emit(curIDSignal, curID);
                plca_active = false;
            );
            FSMA_Transition(T1,
                            plca_en && local_nodeID != 0 && local_nodeID != 255,
                            CS_RESYNC,
            );
            FSMA_Transition(T2,
                            plca_en && local_nodeID == 0,
                            CS_RECOVER,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_RESYNC) {
            FSMA_Enter(
                plca_active = false;
            );
            FSMA_Transition(T1,
                            local_nodeID != 0 && CRS,
                            CS_EARLY_RECEIVE,
            );
            FSMA_Transition(T2,
                            PMCD && !CRS && local_nodeID == 0,
                            CS_SEND_BEACON,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_RECOVER) {
            FSMA_Enter(
                plca_active = false;
            );
            FSMA_Transition(T1,
                            true,
                            CS_WAIT_TO,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T3, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_SEND_BEACON) {
            FSMA_Enter(
                scheduleAfter(20 / 10E+6, beacon_timer);
                tx_cmd = "BEACON";
                emit(txCmdSignal, getCmdCode(tx_cmd));
                FSMA_Delay_Action(phy->startSignalTransmission(BEACON));
                plca_active = true;
            );
            FSMA_Transition(T1,
                            !beacon_timer->isScheduled(),
                            CS_SYNCING,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T3, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
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
                plca_active = true;
                if (local_nodeID != 0 && rx_cmd != "BEACON")
                    rescheduleAfter(4000E-9, invalid_beacon_timer);
            );
            FSMA_Transition(T1,
                            !CRS,
                            CS_WAIT_TO,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T3, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
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
                            plca_active && curID == local_nodeID && packetPending && !CRS,
                            CS_COMMIT,
            );
            FSMA_Transition(T3,
                            !to_timer->isScheduled() && curID != local_nodeID && !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T4,
                            curID == local_nodeID && (!packetPending || !plca_active) && !CRS,
                            CS_YIELD,
            );
            FSMA_Transition(T5, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T6, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_EARLY_RECEIVE) {
            FSMA_Enter(
                cancelEvent(to_timer);
                scheduleAfter(22 / 10E+6, beacon_det_timer);
            );
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
            FSMA_Transition(T5, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T6, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_COMMIT) {
            FSMA_Enter(
                tx_cmd = "COMMIT";
                emit(txCmdSignal, getCmdCode(tx_cmd));
                FSMA_Delay_Action(phy->startSignalTransmission(COMMIT));
                committed = true;
                cancelEvent(to_timer);
                bc = 0;
            );
            FSMA_Transition(T1,
                            TX_EN,
                            CS_TRANSMIT,
            );
            FSMA_Transition(T2,
                            !TX_EN && !packetPending,
                            CS_ABORT,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_YIELD) {
            FSMA_Transition(T1,
                            CRS && to_timer->isScheduled(),
                            CS_EARLY_RECEIVE,
            );
            FSMA_Transition(T2,
                            !to_timer->isScheduled(),
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_RECEIVE) {
            FSMA_Transition(T1,
                            !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T3, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
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
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
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
            );
            FSMA_Transition(T2,
                            !TX_EN && !burst_timer->isScheduled(),
                            CS_ABORT,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_ABORT) {
            FSMA_Enter(
                if (tx_cmd != "NONE") {
                    FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE));
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
            );
            FSMA_Transition(T1,
                            !CRS,
                            CS_NEXT_TX_OPPORTUNITY,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T3, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
        FSMA_State(CS_NEXT_TX_OPPORTUNITY) {
            FSMA_Enter(
                curID = curID + 1;
                emit(curIDSignal, curID);
                committed = false;
            );
            FSMA_Transition(T1, // B
                            (local_nodeID == 0 && curID >= plca_node_count) || curID == 255,
                            CS_RESYNC,
            );
            FSMA_Transition(T2,
                            true,
                            CS_WAIT_TO,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || local_nodeID == 255,
                            CS_DISABLE,
            );
//            FSMA_Transition(T4, // open arrow
//                            !invalid_beacon_timer->isScheduled(),
//                            CS_RESYNC,
//            );
        }
    } }
    controlFsm.executeDelayedActions();
}

void NewEthernetPlca::handleWithDataFSM()
{
    // 148.4.5.7 State diagram of IEEE Std 802.3cg-2019
    { FSMA_Switch(dataFsm) {
        FSMA_State(DS_NORMAL) {
            FSMA_Enter(
                packetPending = false;
                if (CRS)
                    CARRIER_STATUS = "CARRIER_ON";
                else
                    CARRIER_STATUS = "CARRIER_OFF";
                TXD = plca_txd;
                TX_EN = plca_txen;
                TX_ER = plca_txer;
                if (COL)
                    SIGNAL_STATUS = "SIGNAL_ERROR";
                else
                    SIGNAL_STATUS = "NO_SIGNAL_ERROR";
            );
            FSMA_Transition(T1,
                            plca_en && !plca_reset && plca_status,
                            DS_IDLE,
            );
//            FSMA_Transition(T2,
//                            true,
//                            DS_NORMAL,
//            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_WAIT_IDLE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_OFF";
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
//                TXD = ENCODE_TXD(tx_cmd_sync)
                if (TX_EN) {
                    FSMA_Delay_Action(phy->endFrameTransmission());
                    TX_EN = false;
                }
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
//                a = 0;
//                b = 0;
            );
            FSMA_Transition(T1,
                            MCD && !CRS,
                            DS_IDLE,
            );
            FSMA_Transition(T2, // B
                            MCD && CRS && plca_txen,
                            DS_TRANSMIT,
            );
//            FSMA_Transition(T3,
//                            true,
//                            DS_WAIT_IDLE,
//            );
            FSMA_Transition(T4, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_IDLE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_OFF";
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
//                TXD = ENCODE_TXD(tx_cmd_sync)
                TX_EN = false;
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
//                a = 0;
//                b = 0;
            );
            FSMA_Transition(T1,
                            receiving && !plca_txen && tx_cmd == "NONE",
                            DS_RECEIVE,
            );
            FSMA_Transition(T2,
                            plca_txen,
                            DS_HOLD,
            );
//            FSMA_Transition(T3,
//                            true,
//                            DS_IDLE,
//            );
            FSMA_Transition(T4, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_RECEIVE) {
            FSMA_Enter(
                if (CRS && rx_cmd == "COMMIT")
                    CARRIER_STATUS = "CARRIER_ON";
                else
                    CARRIER_STATUS = "CARRIER_OFF";
//                TXD = ENCODE_TXD(tx_cmd_sync)
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
            );
            FSMA_Transition(T1,
                            !receiving && !plca_txen,
                            DS_IDLE,
            );
            FSMA_Transition(T2,
                            plca_txen,
                            DS_COLLIDE,
            );
//            FSMA_Transition(T3,
//                            true,
//                            DS_RECEIVE,
//            );
            FSMA_Transition(T4, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_HOLD) {
            FSMA_Enter(
                packetPending = true;
                CARRIER_STATUS = "CARRIER_ON";
//                a = a + 1;
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
//                TXD = ENCODE_TXD(tx_cmd_sync)
                scheduleAfter(delay_line_length / 10E+6, hold_timer);
            );
//            FSMA_Transition(T1,
//                            MCD && !committed && !plca_txer && !receiving && /*a < delay_line_length*/ hold_timer->isScheduled(),
//                            DS_HOLD,
//            );
            FSMA_Transition(T2,
                            !plca_txer && (receiving || /*a > delay_line_length*/ !hold_timer->isScheduled()),
                            DS_COLLIDE,
            );
            FSMA_Transition(T3,
                            MCD && committed && !receiving && !plca_txer && /*a < delay_line_length*/ hold_timer->isScheduled(),
                            DS_TRANSMIT,
            );
            FSMA_Transition(T4,
                            MCD && plca_txer,
                            DS_ABORT,
            );
            FSMA_Transition(T5, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_ABORT) {
            FSMA_Enter(
                packetPending = false;
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
//                TXD = ENCODE_TXD(tx_cmd_sync)
            );
            FSMA_Transition(T1,
                            !plca_txen,
                            DS_ABORT,
            );
            FSMA_Transition(T2,
                            true,
                            DS_IDLE,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_COLLIDE) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_ON";
                SIGNAL_STATUS = "SIGNAL_ERROR";
//                a = 0;
//                b = 0;
//                TXD = ENCODE_TXD(tx_cmd_sync)
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
                scheduleAfter(512 / 10E+6, pending_timer);
            );
            FSMA_Transition(T1,
                            !plca_txen,
                            DS_DELAY_PENDING,
            );
//            FSMA_Transition(T2,
//                            true,
//                            DS_COLLIDE,
//            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_DELAY_PENDING) {
            FSMA_Enter(
                SIGNAL_STATUS = "NO_SIGNAL_ERROR";
//                TXD = ENCODE_TXD(tx_cmd_sync)
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
            );
            FSMA_Transition(T1,
                            !pending_timer->isScheduled(),
                            DS_PENDING,
            );
//            FSMA_Transition(T2,
//                            true,
//                            DS_DELAY_PENDING,
//            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_PENDING) {
            FSMA_Enter(
                packetPending = true;
                scheduleAfter(288 / 10E+6, commit_timer);
//                TXD = ENCODE_TXD(tx_cmd_sync)
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
            );
            FSMA_Transition(T1,
                            committed,
                            DS_WAIT_MAC,
            );
//            FSMA_Transition(T2,
//                            true,
//                            DS_PENDING,
//            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_WAIT_MAC) {
            FSMA_Enter(
                CARRIER_STATUS = "CARRIER_OFF";
//                TXD = ENCODE_TXD(tx_cmd_sync)
//                TX_ER = ENCODE_TXER(tx_cmd_sync)
            );
            FSMA_Transition(T1,
                            MCD && plca_txen,
                            DS_TRANSMIT,
            );
            FSMA_Transition(T2, // C
                            !plca_txen && !commit_timer->isScheduled(),
                            DS_WAIT_IDLE,
            );
//            FSMA_Transition(T3,
//                            true,
//                            DS_WAIT_MAC,
//            );
            FSMA_Transition(T4, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_TRANSMIT) {
            FSMA_Enter(
                packetPending = false;
                CARRIER_STATUS = "CARRIER_ON";
                if (tx_cmd != "NONE") { // KLUDGE: end commit signal
//                    FSMA_Delay_Action(phy->endSignalTransmission());
                    tx_cmd = "NONE";
                    emit(txCmdSignal, getCmdCode(tx_cmd));
                }
                TXD = plca_txd;
//                FSMA_Delay_Action(phy->startFrameTransmission(plca_txd, false));
                TX_EN = true;
                TX_ER = plca_txer;
                if (COL) {
                    SIGNAL_STATUS = "SIGNAL_ERROR";
//                    a = 0;
                }
                else
                    SIGNAL_STATUS = "NO_SIGNAL_ERROR";
            );
//            FSMA_Transition(T1,
//                            MCD && plca_txen,
//                            DS_TRANSMIT,
//            );
//            FSMA_Transition(T2,
//                            MCD && !plca_txen /*&& a > 0*/,
//                            DS_FLUSH,
//            );
            FSMA_Transition(T3, // C
                            MCD && !plca_txen /*&& a == 0*/,
                            DS_WAIT_IDLE,
            );
            FSMA_Transition(T4, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
        FSMA_State(DS_FLUSH) {
            FSMA_Enter(
                CARRIER_STATUS = "CARRIER_ON";
//                TXD = plca_txdn–a
                TX_EN = true;
                TX_ER = plca_txer;
//                b = b + 1;
                if (COL)
                    SIGNAL_STATUS = "SIGNAL_ERROR";
                else
                    SIGNAL_STATUS = "NO_SIGNAL_ERROR";
            );
//            FSMA_Transition(T1,
//                            MCD && a != b,
//                            DS_FLUSH,
//            );
            FSMA_Transition(T2,
                            MCD /*&& a == b*/,
                            DS_WAIT_IDLE, // C
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en || !plca_status,
                            DS_IDLE,
            );
        }
    } }
    dataFsm.executeDelayedActions();
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

void NewEthernetPlca::handleWithStatusFSM()
{
    // 148.4.6.5 State diagram of IEEE Std 802.3cg-2019
    { FSMA_Switch(statusFsm) {
        FSMA_State(SS_INACTIVE) {
            FSMA_Enter(
                plca_status = false;
            );
            FSMA_Transition(T1,
                            plca_active,
                            SS_ACTIVE,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en,
                            SS_INACTIVE,
            );
        }
        FSMA_State(SS_ACTIVE) {
            FSMA_Enter(
                plca_status = true;
            );
            FSMA_Transition(T1,
                            !plca_active,
                            SS_HYSTERESIS,
            );
            FSMA_Transition(T2, // open arrow
                            plca_reset || !plca_en,
                            SS_INACTIVE,
            );
        }
        FSMA_State(SS_HYSTERESIS) {
            FSMA_Enter(
                rescheduleAfter(130090 / 10E+6, plca_status_timer);
            );
            FSMA_Transition(T1,
                            plca_active,
                            SS_ACTIVE,
            );
            FSMA_Transition(T2,
                            !plca_status_timer->isScheduled() && !plca_active,
                            SS_INACTIVE,
            );
            FSMA_Transition(T3, // open arrow
                            plca_reset || !plca_en,
                            SS_INACTIVE,
            );
        }
    } }
    statusFsm.executeDelayedActions();
}

void NewEthernetPlca::handleWithFSMs()
{
    int controlState, dataState, statusState;
    do {
        controlState = controlFsm.getState();
        dataState = dataFsm.getState();
        statusState = statusFsm.getState();
        handleWithControlFSM();
        handleWithDataFSM();
        handleWithStatusFSM();
    }
    while (controlState != controlFsm.getState() || dataState != dataFsm.getState() || statusState != statusFsm.getState());
}

int NewEthernetPlca::getCmdCode(std::string cmd)
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

