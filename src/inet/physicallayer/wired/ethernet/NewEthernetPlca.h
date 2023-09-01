//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEWETHERNETPLCA_H
#define __INET_NEWETHERNETPLCA_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/basic/IEthernetCsmaMac.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/physicallayer/wired/ethernet/IEthernetCsmaPhy.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements Physical Layer Collision Avoidance (PLCA) from IEEE Std 802.3cg-2019.
 */
class INET_API NewEthernetPlca : public cSimpleModule, public virtual IEthernetCsmaMac, public virtual IEthernetCsmaPhy
{
  public:
    static simsignal_t curIDSignal;
    static simsignal_t controlStateChangedSignal;
    static simsignal_t dataStateChangedSignal;
    static simsignal_t statusStateChangedSignal;
    static simsignal_t rxCmdSignal;
    static simsignal_t txCmdSignal;

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
        DS_NORMAL,
        DS_WAIT_IDLE,
        DS_IDLE,
        DS_RECEIVE,
        DS_HOLD,
        DS_ABORT,
        DS_COLLIDE,
        DS_DELAY_PENDING,
        DS_PENDING,
        DS_WAIT_MAC,
        DS_TRANSMIT,
        DS_FLUSH,
    };

    // 148.4.6.5 State diagram from IEEE Std 802.3cg-2019
    enum StatusState {
        SS_INACTIVE,
        SS_ACTIVE,
        SS_HYSTERESIS,
    };

  protected:
    // parameters which can be configured from the NED/INI files
    bool plca_en = false;
    bool plca_reset = false;
    int plca_node_count = -1;
    int local_nodeID = -1;
    int max_bc = -1;
    int delay_line_length = -1;
    simtime_t to_interval;
    simtime_t burst_interval;

    // environment for external communication
    IEthernetCsmaPhy *phy = nullptr;
    IEthernetCsmaMac *mac = nullptr;

    // state machines
    Fsm controlFsm;
    Fsm dataFsm;
    Fsm statusFsm;

    // 148.4.4.2 PLCA Control variables and 148.4.5.2 Variables and 148.4.6.2 PLCA Status variables
    bool COL = false;
    bool committed = false;
    bool CRS = false;
    bool MCD = true;
    bool packetPending = false;
    bool plca_active = false;
    bool plca_status = true;
    bool plca_txen = false;
    bool plca_txer = false;
    bool PMCD = true;
    bool receiving = false;
    bool RX_DV = false;
    bool TX_EN = false;
    bool TX_ER = false;
//    int a = -1;
//    int b = -1;
    int bc = 0;
    int curID = -1;
    Packet *plca_txd = nullptr;
    Packet *TXD = nullptr;
    std::string CARRIER_STATUS = "CARRIER_OFF";
    std::string rx_cmd = "NONE"; // possible values "NONE", "BEACON", "COMMIT"
    std::string SIGNAL_STATUS = "NO_SIGNAL_ERROR";
    std::string tx_cmd = "NONE"; // possible values "NONE", "BEACON", "COMMIT"

    // additional state variables
    bool old_carrier_sense_signal = false;
    bool old_collision_signal = false;
    EthernetSignalType plca_txtype;

    // control state machine timers
    cMessage *beacon_timer = nullptr;
    cMessage *beacon_det_timer = nullptr;
    cMessage *invalid_beacon_timer = nullptr;
    cMessage *burst_timer = nullptr;
    cMessage *to_timer = nullptr;

    // data state machine timers
    cMessage *hold_timer = nullptr; // additional timer not present in the standard to avoid looping per bit
    cMessage *pending_timer = nullptr;
    cMessage *commit_timer = nullptr;

    // status state machine timers
    cMessage *plca_status_timer = nullptr;

    // statistics

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshDisplay() const override;

    virtual void handleMessage(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message);

    virtual void handleWithControlFSM();
    virtual void handleWithDataFSM();
    virtual void handleWithStatusFSM();

    virtual void handleWithFSMs();

    virtual int getCmdCode(std::string cmd);

  public:
    virtual ~NewEthernetPlca();

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

