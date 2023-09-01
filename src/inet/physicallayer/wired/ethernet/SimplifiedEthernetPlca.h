//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIMPLIFIEDETHERNETPLCA_H
#define __INET_SIMPLIFIEDETHERNETPLCA_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/base/EthernetModes.h"
#include "inet/linklayer/ethernet/basic/IEthernetCsmaMac.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/physicallayer/wired/ethernet/IEthernetCsmaPhy.h"

namespace inet {

namespace physicallayer {

// different approaches for implementing HOLD and PENDING_DELAY
// - add two timers to the current state machine and handle them separately in the handleSelfMessage to avoid drastically increasing the number of transitions
// - add a separate state machine that have the above two states and the related timers, it would delay packetPending = true and also the collision notification to the MAC
// - switch back to the standard based state machines
//   - use the control state machine from the standard as it is already implemented
//   - use the simplified data state machine for which we created a GraphViz diagram


// TODO: Table 147–1—4B/5B Encoding
// TODO: normal transmit opportunity ends with ESDBRS ESDOK signals end of data
// TODO: ESDBRS ESDOK indicates burst is coming
// TODO: ESDBRS ESDOK COMMIT ESD ESDOK indicates burst end
class INET_API SimplifiedEthernetPlca : public cSimpleModule, public virtual IEthernetCsmaMac, public virtual IEthernetCsmaPhy
{
  public:
    static simsignal_t curIDSignal;
    static simsignal_t stateChangedSignal;

    enum State {
        DISABLED,         // operating in CSMA/CD mode, PLCA is disabled
        SENDING_BEACON,   // the beacon signal transmission starts when entering the state and ends when leaving the state
        SENDING_COMMIT,   // the commit signal transmission starts when entering the state and ends when leaving the state
        SENDING_DATA,     // the data frame transmission starts when entering the state and ends when leaving the state
        WAITING_BEACON,   // waiting for the reception start of a beacon signal
        WAITING_COMMIT,   // waiting for the reception start of a commit signal
        WAITING_DATA,     // waiting for the reception start of a data signal
        WAITING_CRS_OFF,  // waiting for the carrier sense to be off
        WAITING_TO,       // waiting for next transmit opportunity
        RECEIVING_BEACON, // the beacon signal reception starts when entering the state and ends when leaving the state
        RECEIVING_COMMIT, // the commit signal reception starts when entering the state and ends when leaving the state
        RECEIVING_DATA,   // the data frame reception starts when entering the state and ends when leaving the state
        END_TO,           // end of transmit opportunity, transient state
    };

    enum Event {
        ENABLE_PLCA,  // switch from CSMA/CD to CSMA/CA using PLCA
        DISABLE_PLCA, // switch from CSMA/CA using PLCA to CSMA/CD
        CARRIER_SENSE_START, // from PHY (transmitting or receiving)
        CARRIER_SENSE_END,   // from PHY (transmitting or receiving)
        COLLISION_START, // from PHY (transmitting and receiving)
        COLLISION_END,   // from PHY (transmitting and receiving)
        RECEPTION_START, // from PHY
        RECEPTION_END,   // from PHY
        START_SIGNAL_TRANSMISSION, // from MAC (JAM)
        END_SIGNAL_TRANSMISSION,   // from MAC
        START_FRAME_TRANSMISSION,  // from MAC for data frames
        END_FRAME_TRANSMISSION,    // from MAC
        END_BEACON_TIMER, // beacon send timer expired
        END_COMMIT_TIMER, // commit send timer expired
        END_DATA_TIMER,   // data send timer expired
        END_TO_TIMER,     // transmit opportunity timer expired
        END_BURST_TIMER,  // burst opportunity timer expired
    };

  protected:
    // parameters
    int plca_node_count = -1; // total number of nodes in the local collision domain
    int local_nodeID = -1;    // node ID for this node
    int max_bc = -1;          // maximum number of packets in a burst using the same transmit opportunity
    simtime_t to_interval;    // transmit opportunity duration before yielding
    simtime_t burst_interval; // amount of time to wait for the MAC to send the next packet in a burst
    simtime_t beacon_duration;
    const EthernetModes::EthernetMode *mode = nullptr;

    // environment
    opp_component_ptr<NetworkInterface> networkInterface;
    opp_component_ptr<IEthernetCsmaPhy> phy;
    opp_component_ptr<IEthernetCsmaMac> mac;

    // state
    Fsm fsm;
    bool packetPending = false; // indicates that the MAC has a packet to send
    bool phyCOL = false; // PHY collision
    bool phyCRS = false; // PHY carrier sense
    int curID = -1; // node ID for the current transmit opportunity
    int bc = 0;     // packet burst counter specifies how many packets have been sent in the current transmit opportunity
    EthernetSignalType receivedSignalType = NONE; // one of BEACON, COMMIT, DATA, or JAM
    EthernetEsdType receivedEsd1 = ESDNONE;

    // timers
    cMessage *beaconTimer = nullptr;
    cMessage *commitTimer = nullptr;
    cMessage *dataTimer = nullptr;
    cMessage *toTimer = nullptr;
    cMessage *burstTimer = nullptr;

    // statistics

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message);
    virtual void refreshDisplay() const override;

    virtual void handleWithFSM(int event, cMessage *message);

    virtual void sendBeacon();
    virtual void waitBeacon();
    virtual void delayTransmission(Packet *packet);
    virtual void setCurID(int value);

  public:
    virtual ~SimplifiedEthernetPlca();

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

