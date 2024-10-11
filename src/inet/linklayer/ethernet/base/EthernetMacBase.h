//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETMACBASE_H
#define __INET_ETHERNETMACBASE_H

#include "inet/common/INETMath.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/FcsMode_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ethernet/base/EthernetModes.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

using namespace inet::queueing;
using namespace inet::physicallayer;

/**
 * Base class for Ethernet MAC implementations.
 */
class INET_API EthernetMacBase : public MacProtocolBase, public queueing::IActivePacketSink
{
  public:
    enum MacTransmitState {
        TX_IDLE_STATE = 1,
        WAIT_IFG_STATE,
        SEND_IFG_STATE,
        TRANSMITTING_STATE,
        JAMMING_STATE,
        BACKOFF_STATE,
        PAUSE_STATE
        // FIXME add TX_OFF_STATE
    };

    enum MacReceiveState {
        RX_IDLE_STATE = 1,
        RECEIVING_STATE,
        RX_COLLISION_STATE,
        RX_RECONNECT_STATE
        // FIXME add RX_OFF_STATE
    };

  protected:
    // Self-message kind values
    enum SelfMsgKindValues {
        ENDIFG = 100,
        ENDRECEPTION,
        ENDBACKOFF,
        ENDTRANSMISSION,
        ENDJAMMING,
        ENDPAUSE
    };

    // configuration
    const char *displayStringTextFormat = nullptr;
    bool sendRawBytes = false;
    bool allowNonstandardBitrate = true;
    FcsMode fcsMode = FCS_MODE_UNDEFINED;
    EthernetModes::EthernetMode curEtherDescr = EthernetModes::nullEthernetMode; // constants for the current Ethernet mode, e.g. txrate
    simtime_t halfBitTime = SIMTIME_ZERO; // transmission time of a half bit
    bool connected = false; // true if connected to a network, set automatically by exploring the network configuration
    bool promiscuous = false; // if true, passes up all received frames
    bool duplexMode = false; // true if operating in full-duplex mode
    bool frameBursting = false; // frame bursting on/off (Gigabit Ethernet)

    // gate pointers, etc.
    PassivePacketSinkRef upperLayerSink;
    cChannel *transmissionChannel = nullptr; // transmission channel
    cGate *physInGate = nullptr; // pointer to the "phys$i" gate
    cGate *physOutGate = nullptr; // pointer to the "phys$o" gate

    // state
    bool channelsDiffer = false; // true when tx and rx channels differ (only one of them exists, or 'datarate' or 'disable' parameters differ) (configuration error, or between changes of tx/rx channels)
    MacTransmitState transmitState = static_cast<MacTransmitState>(-1); // "transmit state" of the MAC
    MacReceiveState receiveState = static_cast<MacReceiveState>(-1); // "receive state" of the MAC
    simtime_t lastTxFinishTime; // time of finishing the last transmission
    int pauseUnitsRequested = 0; // requested pause duration, or zero -- examined at endTx

    // self messages
    cMessage *endTxTimer = nullptr;
    cMessage *endIfgTimer = nullptr;
    cMessage *endPauseTimer = nullptr;

    // RX / TX signals:
    EthernetSignalBase *curTxSignal = nullptr;

    // statistics
    unsigned long numFramesSent = 0;
    unsigned long numFramesReceivedOK = 0;
    unsigned long numBytesSent = 0; // includes Ethernet frame bytes with padding and FCS
    unsigned long numBytesReceivedOK = 0; // includes Ethernet frame bytes with padding and FCS
    unsigned long numFramesFromHL = 0; // packets received from higher layer (LLC or MacRelayUnit)
    unsigned long numDroppedPkFromHLIfaceDown = 0; // packets from higher layer dropped because interface down or not connected
    unsigned long numDroppedIfaceDown = 0; // packets from network layer dropped because interface down or not connected
    unsigned long numDroppedBitError = 0; // frames dropped because of bit errors
    unsigned long numDroppedNotForUs = 0; // frames dropped because destination address didn't match
    unsigned long numFramesPassedToHL = 0; // frames passed to higher layer
    unsigned long numPauseFramesRcvd = 0; // PAUSE frames received from network
    unsigned long numPauseFramesSent = 0; // PAUSE frames sent

    static simsignal_t rxPkOkSignal;
    static simsignal_t txPausePkUnitsSignal;
    static simsignal_t rxPausePkUnitsSignal;

    static simsignal_t transmissionStateChangedSignal;
    static simsignal_t receptionStateChangedSignal;

  public:
    EthernetMacBase();
    virtual ~EthernetMacBase();

    virtual MacAddress getMacAddress() { return networkInterface ? networkInterface->getMacAddress() : MacAddress::UNSPECIFIED_ADDRESS; }

    double getTxRate() { return curEtherDescr.bitrate; }
    bool isActive() { return connected; }

    MacTransmitState getTransmitState() { return transmitState; }
    MacReceiveState getReceiveState() { return receiveState; }

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    virtual void processAtHandleMessageFinished();

    /**
     * Inserts padding in front of FCS and calculate FCS
     */
    void addPaddingAndSetFcs(Packet *packet, B requiredMinByteLength = MIN_ETHERNET_FRAME_BYTES) const;

    // IActivePacketSink:
    virtual queueing::IPassivePacketSource *getProvider(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

  protected:
    // initialization
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initializeFlags();
    virtual void initializeStatistics();

    // finish
    virtual void finish() override;

    /** Checks destination address and drops the frame when frame is not for us; returns true if frame is dropped */
    virtual bool dropFrameNotForUs(Packet *packet, const Ptr<const EthernetMacHeader>& frame);

    /**
     * Calculates datarates, etc. Verifies the datarates on the incoming/outgoing channels,
     * and throws error when they differ and the parameter errorWhenAsymmetric is true.
     */
    virtual void readChannelParameters(bool errorWhenAsymmetric);
    virtual void printParameters();

    // helpers
    virtual void processConnectDisconnect();
    virtual void encapsulate(Packet *packet);
    virtual void decapsulate(Packet *packet);

    /// Verify ethernet packet: check FCS and payload length
    bool verifyCrcAndLength(Packet *packet);

    // MacBase
    virtual void configureNetworkInterface() override;

    // display
    virtual void refreshDisplay() const override;

    // model change related functions
    virtual void receiveSignal(cComponent *src, simsignal_t signalId, cObject *obj, cObject *details) override;
    virtual void refreshConnection();

    void changeTransmissionState(MacTransmitState newState);
    void changeReceptionState(MacReceiveState newState);

    virtual void cutEthernetSignalEnd(EthernetSignalBase *signal, simtime_t duration);
    virtual void txFinished();
};

} // namespace inet

#endif

