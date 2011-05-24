//
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2004 Andras Varga
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

#include "INotifiable.h"
#include "MACAddress.h"
#include "TxNotifDetails.h"

// Forward declarations:
class EtherFrame;
class EtherTraffic;
class InterfaceEntry;
class IPassiveQueue;
class NotificationBoard;

/**
 * Base class for ethernet MAC implementations.
 */
class INET_API EtherMACBase : public cSimpleModule, public INotifiable, public cListener
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
        RX_COLLISION_STATE
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
        NUM_OF_ETHERDESCRS = 4
    };

    struct EtherDescr
    {
        double      txrate;
        int         maxFramesInBurst;
        int64       maxBytesInBurst;      // with IFG and external datas
        int64       frameMinBytes;        // minimal frame length
        int64       frameInBurstMinBytes; // minimal frame length in burst mode, after first frame
        const_simtime_t   halfBitTime;          // transmission time of a half bit
        const_simtime_t   slotTime;             // slot time
        const_simtime_t   shortestFrameDuration;// precalculated from MIN_ETHERNET_FRAME or GIGABIT_MIN_FRAME_WITH_EXT
    };

    class InnerQueue
    {
      public:
        cQueue queue;
        int queueLimit;               // max queue length

        InnerQueue(const char* name=NULL, int limit=0) : queue(name), queueLimit(limit) {}
    };

    class MacQueue
    {
      public:
        InnerQueue * innerQueue;
        IPassiveQueue *extQueue;

        MacQueue() : innerQueue(NULL), extQueue(NULL) {};

        ~MacQueue() { delete innerQueue; };

        bool isEmpty();

        void setExternalQueue(IPassiveQueue *_extQueue)
                { delete innerQueue; innerQueue = NULL; extQueue = _extQueue; };

        void setInternalQueue(const char* name=NULL, int limit=0)
                { delete innerQueue; innerQueue = new InnerQueue(name, limit); extQueue = NULL; };
    };

    MACAddress address;             // own MAC address

    bool connected;                 // true if connected to a network, set automatically by exploring the network configuration
    bool disabled;                  // true if the MAC is disabled, defined by the user
    bool promiscuous;               // if true, passes up all received frames

    // MAC operation modes and parameters
    bool duplexMode;                // channel connecting to MAC is full duplex, i.e. like a switch with 2 half-duplex lines
    bool carrierExtension;          // carrier extension on/off (Gigabit Ethernet)

    bool hasSubscribers;            // only notify if somebody is listening

    bool frameBursting;             // frame bursting on/off (Gigabit Ethernet)
    simtime_t lastTxFinishTime;     // time of finish last transmission

    // states
    MACTransmitState transmitState; // State of the MAC unit transmitting
    MACReceiveState receiveState;   // State of the MAC unit receiving

    // MAC transmission characteristics
    static const EtherDescr etherDescrs[NUM_OF_ETHERDESCRS];
    static const EtherDescr nullEtherDescr;

    const EtherDescr *curEtherDescr;    // Current Ethernet Constants (eg txrate, ...)

    cChannel *transmissionChannel;  // transmission channel

    int pauseUnitsRequested;        // requested pause duration, or zero -- examined at endTx

    MacQueue txQueue;            // output queue

    cGate *physInGate;              // pointer to the "phys$i" gate
    cGate *physOutGate;             // pointer to the "phys$o" gate

    // notification stuff
    InterfaceEntry *interfaceEntry;  // points into IInterfaceTable
    NotificationBoard *nb;
    TxNotifDetails notifDetails;

    EtherFrame *curTxFrame;

    // self messages
    cMessage *endTxMsg, *endIFGMsg, *endPauseMsg;

    // statistics
    int  framesSentInBurst;            // Number of frames send out in current frame burst
    int  bytesSentInBurst;             // Number of bytes transmitted in current frame burst
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

    static simsignal_t txPkBytesSignal;
    static simsignal_t rxPkBytesOkSignal;
    static simsignal_t passedUpPkBytesSignal;
    static simsignal_t txPausePkUnitsSignal;
    static simsignal_t rxPausePkUnitsSignal;
    static simsignal_t rxPkBytesFromHLSignal;
    static simsignal_t droppedPkBytesNotForUsSignal;
    static simsignal_t droppedPkBytesBitErrorSignal;
    static simsignal_t droppedPkBytesIfaceDownSignal;

    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t packetReceivedFromUpperSignal;

  public:
    EtherMACBase();
    virtual ~EtherMACBase();

    virtual MACAddress getMACAddress() {return address;}

  protected:
    //  initialization
    virtual void initialize();
    virtual void initializeFlags();
    virtual void initializeMACAddress();
    virtual void initializeQueueModule();
    virtual void initializeNotificationBoard();
    virtual void initializeStatistics();
    virtual void registerInterface();

    // helpers
    virtual bool checkDestinationAddress(EtherFrame *frame);
    virtual void calculateParameters();
    virtual void printParameters();
    virtual void prepareTxFrame(EtherFrame *frame);

    // finish
    virtual void finish();

    // event handlers
    virtual void processFrameFromUpperLayer(EtherFrame *msg);
    virtual void processMsgFromNetwork(EtherTraffic *msg);
    virtual void processMessageWhenNotConnected(cMessage *msg);
    virtual void processMessageWhenDisabled(cMessage *msg);
    virtual void handleEndIFGPeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndPausePeriod();
    virtual void scheduleEndIFGPeriod();
    virtual void scheduleEndTxPeriod(cPacket *);
    virtual void scheduleEndPausePeriod(int pauseUnits);

    // helpers
    virtual bool checkAndScheduleEndPausePeriod();
    virtual void fireChangeNotification(int type, cPacket *msg);
    virtual void beginSendFrames();
    virtual void frameReceptionComplete(EtherTraffic *frame);
    virtual void processReceivedDataFrame(EtherFrame *frame);
    virtual void processPauseCommand(int pauseUnits);
    virtual void getNextFrameFromQueue();

    // display
    virtual void updateDisplayString();
    virtual void updateConnectionColor(int txState);

    // notifications
    virtual void updateHasSubcribers() = 0;
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    // model change related functions
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj);
    virtual void refreshConnection(bool connected);
};

#endif

