//
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __ETHER_MAC_BASE_H
#define __ETHER_MAC_BASE_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "InterfaceEntry.h"
#include "TxNotifDetails.h"
#include "NotificationBoard.h"

// Self-message kind values
#define ENDIFG             100
#define ENDRECEPTION       101
#define ENDBACKOFF         102
#define ENDTRANSMISSION    103
#define ENDJAMMING         104
#define ENDPAUSE           105
#define ENDAUTOCONFIG      106

// MAC transmit state
#define TX_IDLE_STATE      1
#define WAIT_IFG_STATE     2
#define TRANSMITTING_STATE 3
#define JAMMING_STATE      4
#define BACKOFF_STATE      5
#define PAUSE_STATE        6

// MAC receive states
#define RX_IDLE_STATE      1
#define RECEIVING_STATE    2
#define RX_COLLISION_STATE 3

class IPassiveQueue;

/**
 * Base class for ethernet MAC implementations.
 */
class INET_API EtherMACBase : public cSimpleModule
{
  protected:
    bool connected;                 // true if connected to a network, set automatically by exploring the network configuration
    bool disabled;                  // true if the MAC is disabled, defined by the user
    bool promiscuous;               // if true, passes up all received frames
    MACAddress address;             // own MAC address
    int txQueueLimit;               // max queue length

    // MAC operation modes and parameters
    bool duplexMode;                // channel connecting to MAC is full duplex, i.e. like a switch with 2 half-duplex lines
    bool carrierExtension;          // carrier extension on/off (Gigabit Ethernet)
    bool frameBursting;             // frame bursting on/off (Gigabit Ethernet)

    // MAC transmission characteristics
    double txrate;                  // transmission rate of MAC, bit/s
    double bitTime;                 // precalculated as 1/txrate
    double slotTime;                // slot time
    double interFrameGap;           // IFG
    double jamDuration;             // precalculated as 8*JAM_SIGNAL_BYTES*bitTime
    double shortestFrameDuration;   // precalculated from MIN_ETHERNET_FRAME or GIGABIT_MIN_FRAME_WITH_EXT

    // states
    int  transmitState;             // State of the MAC unit transmitting
    int  receiveState;              // State of the MAC unit receiving
    int  pauseUnitsRequested;       // requested pause duration, or zero -- examined at endTx

    cQueue txQueue;                 // output queue
    IPassiveQueue *queueModule;     // optional module to receive messages from

    // notification stuff
    InterfaceEntry *interfaceEntry;  // points into InterfaceTable
    NotificationBoard *nb;
    TxNotifDetails notifDetails;

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
    cOutVector numFramesSentVector;
    cOutVector numFramesReceivedOKVector;
    cOutVector numBytesSentVector;
    cOutVector numBytesReceivedOKVector;
    cOutVector numDroppedIfaceDownVector;
    cOutVector numDroppedBitErrorVector;
    cOutVector numDroppedNotForUsVector;
    cOutVector numFramesPassedToHLVector;
    cOutVector numPauseFramesRcvdVector;
    cOutVector numPauseFramesSentVector;

  public:
    EtherMACBase();
    virtual ~EtherMACBase();

    long queueLength() {return txQueue.length();}
    MACAddress getMACAddress() {return address;}

  protected:
    //  initialization
    virtual void initialize();
    virtual void initializeTxrate() = 0;
    void initializeFlags();
    void initializeMACAddress();
    void initializeQueueModule();
    void initializeNotificationBoard();
    void initializeStatistics();
    void registerInterface(double txrate);

    // helpers
    bool checkDestinationAddress(EtherFrame *frame);
    void calculateParameters();
    void printParameters();

    // finish
    virtual void finish();

    // event handlers
    virtual void processFrameFromUpperLayer(EtherFrame *msg);
    virtual void processMsgFromNetwork(cMessage *msg);
    virtual void processMessageWhenNotConnected(cMessage *msg);
    virtual void processMessageWhenDisabled(cMessage *msg);
    virtual void handleEndIFGPeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndPausePeriod();
    void scheduleEndIFGPeriod();
    void scheduleEndTxPeriod(cMessage*);
    void scheduleEndPausePeriod(int pauseUnits);

    // helpers
    bool checkAndScheduleEndPausePeriod();
    void fireChangeNotification(int type, cMessage *msg);
    void beginSendFrames();
    void frameReceptionComplete(EtherFrame *frame);
    void processReceivedDataFrame(EtherFrame *frame);
    void processPauseCommand(int pauseUnits);

    // display
    void updateDisplayString();
    void updateConnectionColor(int txState);
};

#endif
