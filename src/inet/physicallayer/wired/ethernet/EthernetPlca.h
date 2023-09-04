//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEWETHERNETPLCA2_H
#define __INET_NEWETHERNETPLCA2_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/base/EthernetModes.h"
#include "inet/linklayer/ethernet/basic/IEthernetCsmaMac.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/physicallayer/wired/ethernet/IEthernetCsmaPhy.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements Physical Layer Collision Avoidance (PLCA) from IEEE Std 802.3cg-2019.
 */
class INET_API EthernetPlca : public cSimpleModule, public virtual IEthernetCsmaMac, public virtual IEthernetCsmaPhy
{
  public:
    static simsignal_t curIDSignal;
    static simsignal_t carrierSenseChangedSignal;
    static simsignal_t collisionChangedSignal;
    static simsignal_t controlStateChangedSignal;
    static simsignal_t dataStateChangedSignal;
    static simsignal_t rxCmdSignal;
    static simsignal_t txCmdSignal;
    static simsignal_t packetPendingDelaySignal;
    static simsignal_t packetIntervalSignal;
    static simsignal_t transmitOpportunityUsedSignal;

    // 148.4.4.6 State diagram from IEEE Std 802.3cg-2019
    enum ControlState {
        CS_DISABLE,
        CS_RESYNC,
        CS_RECOVER,
        CS_SEND_BEACON,
        CS_SYNCING,
        CS_WAIT_TO,
        CS_EARLY_RECEIVE,
        CS_COMMIT,
        CS_YIELD,
        CS_RECEIVE,
        CS_TRANSMIT,
        CS_BURST,
        CS_ABORT,
        CS_NEXT_TX_OPPORTUNITY,
    };

    // 148.4.5.7 State diagram from IEEE Std 802.3cg-2019
    enum DataState {
        DS_WAIT_IDLE,
        DS_IDLE,
        DS_RECEIVE,
        DS_HOLD,
        DS_COLLIDE,
        DS_DELAY_PENDING,
        DS_PENDING,
        DS_WAIT_MAC,
        DS_TRANSMIT,
    };

    enum Event {
        CARRIER_SENSE_START, // from PHY (transmitting or receiving)
        CARRIER_SENSE_END,   // from PHY (transmitting or receiving)
        COLLISION_START, // from PHY (transmitting and receiving)
        COLLISION_END,   // from PHY (transmitting and receiving)
        RECEPTION_START, // from PHY
        RECEPTION_END,   // from PHY
        COMMIT_TO,       // from control FSM
        START_SIGNAL_TRANSMISSION, // from MAC (JAM)
        END_SIGNAL_TRANSMISSION,   // from MAC
        START_FRAME_TRANSMISSION,  // from MAC for data frames
        END_FRAME_TRANSMISSION,    // from MAC
        END_BEACON_TIMER,
        END_BEACON_DET_TIMER,
        END_BURST_TIMER,
        END_TO_TIMER,
        END_HOLD_TIMER,
        END_PENDING_TIMER,
        END_COMMIT_TIMER,
        END_TX_TIMER,
    };

    enum CARRIER_STATUS_ENUM {CARRIER_OFF, CARRIER_ON};
    enum SIGNAL_STATUS_ENUM { NO_SIGNAL_ERROR, SIGNAL_ERROR };
    enum CMD_ENUM {CMD_NONE, CMD_BEACON, CMD_COMMIT};

  protected:
    // parameters which can be configured from the NED/INI files
    int plca_node_count = -1;
    int local_nodeID = -1;
    int max_bc = -1;
    int delay_line_length = -1;
    simtime_t to_interval = -1;
    simtime_t burst_interval = -1;

    // environment for external communication
    IEthernetCsmaPhy *phy = nullptr;
    IEthernetCsmaMac *mac = nullptr;

    // state machines
    Fsm controlFsm;
    Fsm dataFsm;

    // 148.4.4.2 PLCA Control variables and 148.4.5.2 Variables and 148.4.6.2 PLCA Status variables
    bool COL = false;
    bool committed = false;
    bool CRS = false;
//    bool MCD = true;
    bool packetPending = false;
//    bool plca_txen = false;
//    bool plca_txer = false;
    bool PMCD = true;
    bool receiving = false;
    bool RX_DV = false;
    bool TX_EN = false;
//    bool TX_ER = false;
//    int a = -1;
//    int b = -1;
    int bc = 0;
    int curID = -1;
    Packet *currentTx = nullptr;
    CARRIER_STATUS_ENUM CARRIER_STATUS = CARRIER_OFF;
    CMD_ENUM rx_cmd = CMD_NONE;
    SIGNAL_STATUS_ENUM SIGNAL_STATUS = NO_SIGNAL_ERROR;
    CMD_ENUM tx_cmd = CMD_NONE;

    // additional state variables
    const EthernetModes::EthernetMode *mode = nullptr;
    simtime_t macStartFrameTransmissionTime = -1;
    simtime_t phyStartFrameTransmissionTime = -1;
    bool old_carrier_sense_signal = false;
    bool old_collision_signal = false;

    // control state machine timers
    cMessage *beacon_timer = nullptr;
    cMessage *beacon_det_timer = nullptr;
    cMessage *burst_timer = nullptr;
    cMessage *to_timer = nullptr;

    // data state machine timers
    cMessage *hold_timer = nullptr; // additional timer not present in the standard to avoid looping per bit
    cMessage *pending_timer = nullptr;
    cMessage *commit_timer = nullptr;
    cMessage *tx_timer = nullptr; // additional timer not present in the standard to avoid looping per bit

    // statistics

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void handleMessage(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message);

    virtual void handleWithControlFSM();
    virtual void handleWithDataFSM(int event, cMessage *message);

  public:
    virtual ~EthernetPlca();

    virtual void handleCarrierSenseStart() override;
    virtual void handleCarrierSenseEnd() override;

    virtual void handleCollisionStart() override;
    virtual void handleCollisionEnd() override;

    virtual void handleReceptionStart(EthernetSignalType signalType, Packet *packet) override;
    virtual void handleReceptionError(EthernetSignalType signalType, Packet *packet) override;
    virtual void handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1) override;

    virtual b getEsdLength() override { return B(1); }

    virtual void startSignalTransmission(EthernetSignalType signalType) override;
    virtual void endSignalTransmission(EthernetEsdType esd1) override;

    virtual void startFrameTransmission(Packet *packet, EthernetEsdType esd1) override;
    virtual void endFrameTransmission() override;
};

} // namespace physicallayer

} // namespace inet

#endif

