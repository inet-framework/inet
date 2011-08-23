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

#ifndef __INET_ETHER_MAC_BASE_H
#define __INET_ETHER_MAC_BASE_H

#include "INETDefs.h"

#include "IPassiveQueue.h"
#include "MACAddress.h"

// Forward declarations:
class EtherFrame;
class EtherTraffic;
class InterfaceEntry;

/**
 * Base class for ethernet MAC implementations.
 */
class INET_API EtherMACBase : public cSimpleModule, public cListener
{
  protected:
    enum MACTransmitState
    {
        TX_IDLE_STATE = 1,
        WAIT_IFG_STATE,
        SEND_IFG_STATE,
        TRANSMITTING_STATE,
        JAMMING_STATE,
        BACKOFF_STATE,
        PAUSE_STATE
    };

    enum MACReceiveState
    {
        RX_IDLE_STATE = 1,
        RECEIVING_STATE,
        RX_COLLISION_STATE,
        RX_RECONNECT_STATE
    };

    // Self-message kind values
    enum SelfMsgKindValues
    {
        ENDIFG = 100,
        ENDRECEPTION,
        ENDBACKOFF,
        ENDTRANSMISSION,
        ENDJAMMING,
        ENDPAUSE
    };

    enum
    {
        NUM_OF_ETHERDESCRS = 6
    };

    struct EtherDescr
    {
        double        txrate;
        double        halfBitTime;          // transmission time of a half bit
        int64         frameMinBytes;        // minimal frame length
        //for half duplex:
        unsigned int  maxFramesInBurst;
        int64         maxBytesInBurst;      // with IFG and external datas
        int64         halfDuplexFrameMinBytes;   // minimal frame length in half duplex mode, half duplex not supported when smaller than 0
        int64         frameInBurstMinBytes; // minimal frame length in burst mode, after first frame
        double        slotTime;             // slot time
        double        maxPropagationDelay;  // used for detecting longer cables than allowed
    };

    class InnerQueue
    {
      protected:
        cQueue queue;
        int queueLimit;               // max queue length

      protected:
        static int packetCompare(cObject *a, cObject *b);  // PAUSE frames have higher priority

      public:
        InnerQueue(const char* name = NULL, int limit = 0) : queue(name, packetCompare), queueLimit(limit) {}
        void insertFrame(cObject *obj) { queue.insert(obj); }
        cObject *pop() { return queue.pop(); }
        bool empty() const { return queue.empty(); }
        int getQueueLimit() const { return queueLimit; }
        bool isFull() const { return queueLimit != 0 && queue.length() > queueLimit; }
        int length() const { return queue.length(); }
    };

    class MacQueue
    {
      public:
        InnerQueue * innerQueue;
        IPassiveQueue *extQueue;

      public:
        MacQueue() : innerQueue(NULL), extQueue(NULL) {};
        ~MacQueue() { delete innerQueue; };
        bool isEmpty() { return innerQueue ? innerQueue->empty() : extQueue->isEmpty(); }
        void setExternalQueue(IPassiveQueue *_extQueue)
                { delete innerQueue; innerQueue = NULL; extQueue = _extQueue; };
        void setInternalQueue(const char* name = NULL, int limit = 0)
                { delete innerQueue; innerQueue = new InnerQueue(name, limit); extQueue = NULL; };
    };

    MACAddress address;             // own MAC address

    bool connected;                 // true if connected to a network, set automatically by exploring the network configuration
    bool disabled;                  // true if the MAC is disabled, defined by the user
    bool promiscuous;               // if true, passes up all received frames

    bool dataratesDiffer;           // true when tx rate and rx rate differ (configuration error, or between datarate change of tx/rx channels)

    // MAC operation modes and parameters
    // TODO: some of these parameters do not have any meaning for EtherMACFullDuplex, they should rather be in EtherMAC instead
    bool duplexMode;                // channel connecting to MAC is full duplex, i.e. like a switch with 2 half-duplex lines

    bool frameBursting;             // frame bursting on/off (Gigabit Ethernet)
    simtime_t lastTxFinishTime;     // time of finish last transmission

    // states
    MACTransmitState transmitState; // State of the MAC unit transmitting
    MACReceiveState receiveState;   // State of the MAC unit receiving

    // MAC transmission characteristics
    static const EtherDescr etherDescrs[NUM_OF_ETHERDESCRS];
    static const EtherDescr nullEtherDescr;

    const EtherDescr *curEtherDescr;       // Current Ethernet Constants (eg txrate, ...)

    cChannel *transmissionChannel;  // transmission channel

    int pauseUnitsRequested;        // requested pause duration, or zero -- examined at endTx

    MacQueue txQueue;               // output queue

    cGate *physInGate;              // pointer to the "phys$i" gate
    cGate *physOutGate;             // pointer to the "phys$o" gate
    cGate *upperLayerInGate;        // pointer to the "upperLayerIn" gate

    // notification stuff
    InterfaceEntry *interfaceEntry;  // points into IInterfaceTable

    EtherFrame *curTxFrame;

    // self messages
    cMessage *endTxMsg, *endIFGMsg, *endPauseMsg;

    // statistics
    unsigned long numFramesSent;
    unsigned long numFramesReceivedOK;
    unsigned long numBytesSent;        // includes Ethernet frame bytes with preamble
    unsigned long numBytesReceivedOK;  // includes Ethernet frame bytes with preamble
    unsigned long numFramesFromHL;     // packets received from higer layer (LLC or MACRelayUnit)
    unsigned long numDroppedIfaceDown; // packets from higher layer dropped because interface down (TBD not impl yet)
    unsigned long numDroppedBitError;  // frames dropped because of bit errors
    unsigned long numDroppedNotForUs;  // frames dropped because destination address didn't match
    unsigned long numFramesPassedToHL; // frames passed to higher layer
    unsigned long numPauseFramesRcvd;  // PAUSE frames received from network
    unsigned long numPauseFramesSent;  // PAUSE frames sent

    static simsignal_t txPkSignal;
    static simsignal_t rxPkOkSignal;
    static simsignal_t txPausePkUnitsSignal;
    static simsignal_t rxPausePkUnitsSignal;
    static simsignal_t rxPkFromHLSignal;
    static simsignal_t dropPkNotForUsSignal;
    static simsignal_t dropPkBitErrorSignal;
    static simsignal_t dropPkIfaceDownSignal;

    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t packetReceivedFromUpperSignal;

  public:
    static const double SPEED_OF_LIGHT_IN_CABLE;

  public:
    EtherMACBase();
    virtual ~EtherMACBase();

    virtual MACAddress getMACAddress() {return address;}

    double getTxRate() { return curEtherDescr->txrate; }
    bool isActive() { return connected && !disabled; }

  protected:
    //  initialization
    virtual void initialize();
    virtual void initializeFlags();
    virtual void initializeMACAddress();
    virtual void initializeQueueModule();
    virtual void initializeStatistics();
    virtual void registerInterface();

    // helpers
    /** Checks destination address and drop frame when not came for me */
    virtual bool dropFrameNotForUs(EtherFrame *frame);

    /**
     * Calculates datarates, etc. Verify the same settings on in/out channels, and throw error
     * when differs and the parameter errorWhenAsymmetric is true.
     */
    virtual void calculateParameters(bool errorWhenAsymmetric);

    virtual void printParameters();

    // finish
    virtual void finish();

    // event handlers

    // helpers
    virtual void getNextFrameFromQueue();
    virtual void requestNextFrameFromExtQueue();
    virtual void processConnectionChanged();

    // display
    virtual void updateDisplayString();
    virtual void updateConnectionColor(int txState);

    // model change related functions
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
    virtual void refreshConnection();
};

#endif

