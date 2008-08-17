/* -*- mode:c++ -*- ********************************************************
 * file:        Mac80211.h
 *
 * author:      David Raguin/Marc L�bbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef MAC_80211_H
#define MAC_80211_H

#include <list>
#include "WirelessMacBase.h"
#include "Mac80211Pkt_m.h"
#include "Consts80211.h"
#include "NotificationBoard.h"
#include "RadioState.h"


/**
 * @brief An implementation of the 802.11b MAC.
 *
 * For more info, see the NED file.
 *
 * @ingroup macLayer
 * @author David Raguin
 */
class INET_API Mac80211 : public WirelessMacBase, public INotifiable
{
    typedef std::list<Mac80211Pkt*> MacPktList;

    /** Definition of the timer types */
    enum timerType {
      TIMEOUT,
      NAV,
      CONTENTION,
      END_TRANSMISSION,
      END_SIFS
    };

    /** Definition of the states*/
    enum State {
      WFDATA = 0, // waiting for data packet
      QUIET = 1,  // waiting for the communication between two other nodes to end
      IDLE = 2,   // no packet to send, no packet receiving
      CONTEND = 3,// contention state (battle for the channel)
      WFCTS = 4,  // RTS sent, waiting for CTS
      WFACK = 5,  // DATA packet sent, waiting for ACK
      BUSY = 6    // during transmission of an ACK or a BROADCAST packet
    };

  public:
    Mac80211();
    virtual ~Mac80211();

  protected:
    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** @brief Register the interface in IInterfaceTable */
    virtual void registerInterface();

  protected:
    /** @brief Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle packets from upper layer */
    virtual void handleUpperMsg(cPacket*);

    /** @brief Handle commands from upper layer */
    virtual void handleCommand(cMessage*);

    /** @brief Handle packets from lower layer */
    virtual void handleLowerMsg(cPacket*);

    /** @brief handle end of contention */
    virtual void handleEndContentionTimer();
    /** @brief handle a message that is not for me or errornous*/
    virtual void handleMsgNotForMe(Mac80211Pkt*);
    /** @brief handle a message that was meant for me*/
    virtual void handleMsgForMe(Mac80211Pkt*);
    // ** @brief handle a Broadcast message*/
    virtual void handleBroadcastMsg(Mac80211Pkt*);

    /** @brief handle the end of a transmission...*/
    virtual void handleEndTransmissionTimer();

    /** @brief handle end of SIFS*/
    virtual void handleEndSifsTimer();
    /** @brief handle time out*/
    virtual void handleTimeoutTimer();
    /** @brief NAV timer expired, the exchange of messages of other
       stations is done*/
    virtual void handleNavTimer();

    virtual void handleRTSframe(Mac80211Pkt*);

    virtual void handleDATAframe(Mac80211Pkt*);

    virtual void handleACKframe(Mac80211Pkt*);

    virtual void handleCTSframe(Mac80211Pkt*);

    /** @brief send data frame */
    virtual void sendDATAframe();

    /** @brief send Acknoledgement */
    virtual void sendACKframe(Mac80211Pkt*);

    /** @brief send CTS frame */
    virtual void sendCTSframe(Mac80211Pkt*);

    /** @brief send RTS frame */
    virtual void sendRTSframe();

    /** @brief send broadcast frame */
    virtual void sendBROADCASTframe();

    /** @brief encapsulate packet */
    virtual Mac80211Pkt* encapsMsg(cPacket *netw);

    /** @brief decapsulate packet and send to higher layer */
    virtual void decapsulateAndSendUp(Mac80211Pkt *frame);

    /** @brief build a data frame */
    virtual Mac80211Pkt* buildDATAframe();

    /** @brief build an ACK */
    virtual Mac80211Pkt* buildACKframe(Mac80211Pkt*);

    /** @brief build a CTS frame*/
    virtual Mac80211Pkt* buildCTSframe(Mac80211Pkt*);

    /** @brief build an RTS frame */
    virtual Mac80211Pkt* buildRTSframe();

    /** @brief build a broadcast frame*/
    virtual Mac80211Pkt* buildBROADCASTframe();

    /** @brief start a new contention period */
    virtual void beginNewCycle();

    /** @brief Compute a backoff value */
    virtual simtime_t computeBackoff();

    /** @brief Compute a new contention window */
    virtual int computeContentionWindow();

    /** @brief Test if maximum number of retries to transmit is exceeded */
    virtual void testMaxAttempts();

    /** @brief return a timeOut value for a certain type of frame*/
    virtual simtime_t computeTimeout(_802_11frameType type, simtime_t last_frame_duration);

    /** @brief computes the duration of a transmission over the physical channel*/
    virtual simtime_t computePacketDuration(int bits);

    /** @brief Produce a readable name of the given state */
    static const char *stateName(State state);

    /** @brief Produce a readable name of the given timer type */
    static const char *timerTypeName(int type);

    /** @brief Produce a readable name of the given packet type */
    static const char *pktTypeName(int type);

    /** @brief Sets the state, and produces a log message in between */
    virtual void setState(State state);

  protected:
    /** @brief mac address */
    MACAddress myMacAddr;

    // TIMERS:

    /** @brief Timer used for time-outs after the transmission of a RTS,
       a CTS, or a DATA packet*/
    cMessage* timeout;

    /** @brief Timer used for the defer time of a node. Also called NAV :
       networks allocation vector*/
    cMessage* nav;

    /** @brief Timer used for contention periods*/
    cMessage* contention;

    /** @brief Timer used to indicate the end of the transmission of an
       ACK or a BROADCAST packet*/
    cMessage* endTransmission;

    /** @brief Timer used to indicate the end of a SIFS*/
    cMessage* endSifs;

    /** @brief extended interframe space*/
    simtime_t EIFS;

    /** @brief Variable to store the current backoff value*/
    simtime_t BW;

    /** @brief Current state of the MAC*/
    State state;

    /** @brief Current state of the radio (kept updated by receiveChangeNotification()) */
    RadioState::State radioState;

    /** @brief Maximal number of packets in the queue; should be set in
       the omnetpp.ini*/
    int maxQueueSize;

    /** @brief Boolean used to know if the next packet is a broadcast packet.*/
    bool nextIsBroadcast;

    /** @brief Buffering of messages from upper layer*/
    MacPktList fromUpperLayer;

    /** @brief Number of frame transmission attempt*/
    int retryCounter;

    /** @brief If there's a new packet to send and the channel is free, no
        backoff is needed.*/
    bool tryWithoutBackoff;

    /** @brief true if Rts/Cts is used, false if not; can be set in omnetpp.ini*/
    bool rtsCts;

    /** @brief Very small value used in timer scheduling in order to avoid
       multiple changements of state in the same simulation time.*/
    simtime_t delta;

    /** @brief The bitrate should be set in omnetpp.ini; be sure to use a
       valid 802.11 bitrate*/
    double bitrate;

    /** @brief Should be set in the omnetpp.ini*/
    int broadcastBackoff;
};

#endif

