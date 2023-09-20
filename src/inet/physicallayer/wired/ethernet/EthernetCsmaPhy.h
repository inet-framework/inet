//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCSMAPHY_H
#define __INET_ETHERNETCSMAPHY_H

#include "inet/common/FSMA.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/base/EthernetModes.h"
#include "inet/linklayer/ethernet/basic/IEthernetCsmaMac.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/physicallayer/wired/ethernet/IEthernetCsmaPhy.h"

namespace inet {

namespace physicallayer {

class INET_API EthernetCsmaPhy : public cSimpleModule, public virtual IEthernetCsmaPhy
{
  public:
    static simsignal_t stateChangedSignal;
    static simsignal_t receivedSignalTypeSignal;
    static simsignal_t transmittedSignalTypeSignal;
    static simsignal_t busUsedSignal;

    enum State {
        IDLE,         // neither transmitting nor receiving any signal
        TRANSMITTING,
        RECEIVING,    // receiving one or more signals, not necessary successfully
        COLLISION,    // both transmitting and receiving simultaneously up to the point where both end
        CRS_ON,
    };

  protected:
    enum Event {
        TX_START,
        TX_END,
        RX_START,  // start of a received signal
        RX_UPDATE, // update for a received signal
        RX_END_TIMER, // end of the currently received signal
        CRS_OFF_TIMER,
    };

    class INET_API RxSignal {
      public:
        EthernetSignalBase *signal = nullptr;
        simtime_t rxEndTime;
        RxSignal(EthernetSignalBase *signal, simtime_t_cref rxEndTime) : signal(signal), rxEndTime(rxEndTime) {}
    };

  protected:
    // parameters
    const EthernetModes::EthernetMode *mode = nullptr;

    // environment
    cGate *physInGate = nullptr;
    cGate *physOutGate = nullptr;
    cGate *upperLayerInGate = nullptr;
    cGate *upperLayerOutGate = nullptr;
    opp_component_ptr<IEthernetCsmaMac> mac;
    opp_component_ptr<NetworkInterface> networkInterface;

    // state
    Fsm fsm;
    std::vector<RxSignal> rxSignals;
    EthernetSignalBase *currentRxSignal = nullptr;
    EthernetSignalBase *currentTxSignal = nullptr;
    simtime_t txEndTime = -1;

    // timers
    cMessage *rxEndTimer = nullptr; // scheduled with highest priority to the end of all currently received signals
    cMessage *crsOffTimer = nullptr; // scheduled with lowest priority to the end of all currently received signals or currently transmitted signal

    // statistics

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void handleMessage(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message);
    virtual void handleEthernetSignal(EthernetSignalBase *message);
    virtual void handleUpperPacket(Packet *packet);

    virtual void handleWithFsm(int event, cMessage *message);

    virtual void handleStartTransmission(EthernetSignalBase *signal);
    virtual void handleEndTransmission();

    virtual void handleStartReception(EthernetSignalBase *signal);
    virtual void handleEndReception();

    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

    virtual void truncateSignal(EthernetSignalBase *signal, simtime_t duration);

    virtual void updateRxSignals(EthernetSignalBase *signal);
    virtual simtime_t getMaxRxEndTime();

    virtual void scheduleRxEndTimer();
    virtual void scheduleCrsOffTimer();

    virtual void startJamSignalTransmission();
    virtual void startBeaconSignalTransmission();
    virtual void startCommitSignalTransmission();

  public:
    virtual ~EthernetCsmaPhy();

    virtual b getEsdLength() override { return b(0); }

    virtual void startSignalTransmission(EthernetSignalType signalType) override;
    virtual void endSignalTransmission(EthernetEsdType esd1) override;

    virtual void startFrameTransmission(Packet *packet, EthernetEsdType esd1) override;
    virtual void endFrameTransmission() override;
};

} // namespace physicallayer

} // namespace inet

#endif

