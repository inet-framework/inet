//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ETHERMACBASE_H
#define __INET_ETHERMACBASE_H

#include "inet/common/INETDefs.h"

#include "inet/common/INETMath.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/linklayer/base/MACBase.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

// Forward declarations:
class EtherFrame;
class EtherTraffic;
class InterfaceEntry;

/**
 * Base class for Ethernet MAC implementations.
 */
class INET_API EtherMACBase : public MACBase
{
  public:
        enum MACTransmitState {
            TX_IDLE_STATE = 1,
            WAIT_IFG_STATE,
            SEND_IFG_STATE,
            TRANSMITTING_STATE,
            JAMMING_STATE,
            BACKOFF_STATE,
            PAUSE_STATE
        };

        enum MACReceiveState {
            RX_IDLE_STATE = 1,
            RECEIVING_STATE,
            RX_COLLISION_STATE,
            RX_RECONNECT_STATE
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

    enum {
        NUM_OF_ETHERDESCRS = 6
    };

    struct EtherDescr
    {
        double txrate;
        double halfBitTime;    // transmission time of a half bit
        int64 frameMinBytes;    // minimal frame length
        // for half-duplex operation:
        unsigned int maxFramesInBurst;
        int64 maxBytesInBurst;    // including IFG and preamble, etc.
        int64 halfDuplexFrameMinBytes;    // minimal frame length in half-duplex mode; -1 means half duplex is not supported
        int64 frameInBurstMinBytes;    // minimal frame length in burst mode, after first frame
        double slotTime;    // slot time
        double maxPropagationDelay;    // used for detecting longer cables than allowed
    };

    class InnerQueue
    {
      protected:
        cQueue queue;
        int queueLimit = 0;    // max queue length

      protected:
        static int packetCompare(cObject *a, cObject *b);    // PAUSE frames have higher priority

      public:
        InnerQueue(const char *name = nullptr, int limit = 0) : queue(name, packetCompare), queueLimit(limit) {}
        void insertFrame(cObject *obj) { queue.insert(obj); }
        cObject *pop() { return queue.pop(); }
        bool isEmpty() const { return queue.isEmpty(); }
        int getQueueLimit() const { return queueLimit; }
        bool isFull() const { return queueLimit != 0 && queue.getLength() > queueLimit; }
        int getLength() const { return queue.getLength(); }
        void clear() { queue.clear(); }
    };

    class MacQueue
    {
      public:
        InnerQueue *innerQueue = nullptr;
        IPassiveQueue *extQueue = nullptr;

      public:
        ~MacQueue() { delete innerQueue; };
        bool isEmpty() { return innerQueue ? innerQueue->isEmpty() : extQueue->isEmpty(); }
        void setExternalQueue(IPassiveQueue *_extQueue)
        { delete innerQueue; innerQueue = nullptr; extQueue = _extQueue; };
        void setInternalQueue(const char *name = nullptr, int limit = 0)
        { delete innerQueue; innerQueue = new InnerQueue(name, limit); extQueue = nullptr; };
    };

    // MAC constants for bitrates and modes
    static const EtherDescr etherDescrs[NUM_OF_ETHERDESCRS];
    static const EtherDescr nullEtherDescr;

    // configuration
    const EtherDescr *curEtherDescr = nullptr;    // constants for the current Ethernet mode, e.g. txrate
    MACAddress address;    // own MAC address
    bool connected = false;    // true if connected to a network, set automatically by exploring the network configuration
    bool disabled = false;    // true if the MAC is disabled, defined by the user
    bool promiscuous = false;    // if true, passes up all received frames
    bool duplexMode = false;    // true if operating in full-duplex mode
    bool frameBursting = false;    // frame bursting on/off (Gigabit Ethernet)
    bool connectionColoring = false;    // colors the connection when transmitting

    // gate pointers, etc.
    MacQueue txQueue;    // the output queue
    cChannel *transmissionChannel = nullptr;    // transmission channel
    cGate *physInGate = nullptr;    // pointer to the "phys$i" gate
    cGate *physOutGate = nullptr;    // pointer to the "phys$o" gate
    cGate *upperLayerInGate = nullptr;    // pointer to the "upperLayerIn" gate

    // state
    bool channelsDiffer = false;    // true when tx and rx channels differ (only one of them exists, or 'datarate' or 'disable' parameters differ) (configuration error, or between changes of tx/rx channels)
    MACTransmitState transmitState = (MACTransmitState)-1;    // "transmit state" of the MAC
    MACReceiveState receiveState = (MACReceiveState)-1;    // "receive state" of the MAC
    simtime_t lastTxFinishTime;    // time of finishing the last transmission
    int pauseUnitsRequested = 0;    // requested pause duration, or zero -- examined at endTx
    EtherFrame *curTxFrame = nullptr;    // frame being transmitted

    // self messages
    cMessage *endTxMsg = nullptr, *endIFGMsg = nullptr, *endPauseMsg = nullptr;

    // statistics
    unsigned long numFramesSent = 0;
    unsigned long numFramesReceivedOK = 0;
    unsigned long numBytesSent = 0;    // includes Ethernet frame bytes with preamble
    unsigned long numBytesReceivedOK = 0;    // includes Ethernet frame bytes with preamble
    unsigned long numFramesFromHL = 0;    // packets received from higher layer (LLC or MACRelayUnit)
    unsigned long numDroppedPkFromHLIfaceDown = 0;    // packets from higher layer dropped because interface down or not connected
    unsigned long numDroppedIfaceDown = 0;    // packets from network layer dropped because interface down or not connected
    unsigned long numDroppedBitError = 0;    // frames dropped because of bit errors
    unsigned long numDroppedNotForUs = 0;    // frames dropped because destination address didn't match
    unsigned long numFramesPassedToHL = 0;    // frames passed to higher layer
    unsigned long numPauseFramesRcvd = 0;    // PAUSE frames received from network
    unsigned long numPauseFramesSent = 0;    // PAUSE frames sent

    static simsignal_t txPkSignal;
    static simsignal_t rxPkOkSignal;
    static simsignal_t txPausePkUnitsSignal;
    static simsignal_t rxPausePkUnitsSignal;
    static simsignal_t rxPkFromHLSignal;
    static simsignal_t dropPkNotForUsSignal;
    static simsignal_t dropPkBitErrorSignal;
    static simsignal_t dropPkIfaceDownSignal;
    static simsignal_t dropPkFromHLIfaceDownSignal;

    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t packetReceivedFromUpperSignal;

    static simsignal_t transmitStateSignal;
    static simsignal_t receiveStateSignal;

  public:
    static const double SPEED_OF_LIGHT_IN_CABLE;

  public:
    EtherMACBase();
    virtual ~EtherMACBase();

    virtual MACAddress getMACAddress() { return address; }

    double getTxRate() { return curEtherDescr->txrate; }
    bool isActive() { return connected && !disabled; }

    MACTransmitState getTransmitState(){ return transmitState; }
    MACReceiveState getReceiveState(){ return receiveState; }

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:
    //  initialization
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initializeFlags();
    virtual void initializeMACAddress();
    virtual void initializeQueueModule();
    virtual void initializeStatistics();

    // finish
    virtual void finish() override;

    /** Checks destination address and drops the frame when frame is not for us; returns true if frame is dropped */
    virtual bool dropFrameNotForUs(EtherFrame *frame);

    /**
     * Calculates datarates, etc. Verifies the datarates on the incoming/outgoing channels,
     * and throws error when they differ and the parameter errorWhenAsymmetric is true.
     */
    virtual void readChannelParameters(bool errorWhenAsymmetric);
    virtual void printParameters();

    // helpers
    virtual void getNextFrameFromQueue();
    virtual void requestNextFrameFromExtQueue();
    virtual void processConnectDisconnect();

    // MACBase
    virtual InterfaceEntry *createInterfaceEntry() override;
    virtual void flushQueue() override;
    virtual void clearQueue() override;
    virtual bool isUpperMsg(cMessage *msg) override { return msg->getArrivalGate() == upperLayerInGate; }

    // display
    virtual void updateDisplayString();
    virtual void updateConnectionColor(int txState);

    // model change related functions
    virtual void receiveSignal(cComponent *src, simsignal_t signalId, cObject *obj DETAILS_ARG) override;
    virtual void refreshConnection();
};

} // namespace inet

#endif // ifndef __INET_ETHERMACBASE_H

