/* -*- mode:c++ -*- ********************************************************
 * file:        CSMAMacLayer.h
 *
 * author:      Marc Loebbers, Yosia Hadisusanto
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
 ***************************************************************************/


#ifndef CSMAMAC_LAYER_H
#define CSMAMAC_LAYER_H

#include <list>
#include "WirelessMacBase.h"
#include "NotificationBoard.h"
#include "RadioState.h"
#include "MacPkt_m.h"



/**
 * @class CSMAMacLayer
 * @brief MAC module which provides non-persistent CSMA
 *
 * This is an implementation of a simple non-persistent CSMA. The
 * idea of nonpersistent CSMA is "listen before talk". Before
 * attempting to transmit, a station will sense the medium for a
 * carrier signal, which, if present, indicates that some other
 * station is sending.
 *
 * If the channel is busy a random waiting time is computed and after
 * this time the channel is sensed again. Once the channel gets idle
 * the message is sent. (State of the channel is obtained from SnrEval
 * via NotificationBoard.)
 *
 * An option of this module is to use a queue. If a packet from the
 * upper layer arrives although there is still another packet waiting
 * for its transmission or being transmitted the new packet can be
 * stored in this queue. The length of this queue can be specified by
 * the user in the omnetpp.ini file. By default the length is 0. If
 * the queue is full or there is no queue (length = 0) new packet(s)
 * will be deleted.
 *
 * @todo Inform upper layers about the full queue!
 *
 * ATTENTION: Imagine the following scenario:
 *
 * Several stations receive a broadcast request packet, usally exactly
 * at the same time. They will all try to answer at exactly the same
 * time, i.e. they all will sense the channel at exactly the same time
 * and start to transmit because the channel was sensed idle by all of
 * them. Therefore a small random delay should be built into one/some
 * of the upper layers to simulate a random processing time!
 *
 * The TestApplLayer e.g. implements such a processing delay!
 *
 * @ingroup macLayer
 * @author Marc L�bbers, Yosia Hadisusanto
 */
class INET_API CSMAMacLayer : public WirelessMacBase, public INotifiable
{
  public:
    CSMAMacLayer();
    virtual ~CSMAMacLayer();

  protected:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Register the interface in InterfaceTable */
    virtual void registerInterface();

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle packets from lower layer */
    virtual void handleLowerMsg(cPacket*);

    /** @brief Handle commands from upper layer */
    virtual void handleCommand(cMessage*);

    /** @brief Handle packets from upper layer */
    virtual void handleUpperMsg(cPacket*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Encapsulate the given higher-layer packet into MacPkt */
    virtual MacPkt *encapsMsg(cPacket *netw);

    /** @brief Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

  protected:
    /** @brief mac address */
    MACAddress myMacAddr;

    /** @brief Current state of the radio (kept updated by receiveChangeNotification()) */
    RadioState::State radioState;

    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    cQueue macQueue;

    /** @brief length of the queue*/
    int queueLength;

    /** @brief Timer for backoff in case the channel is busy*/
    cMessage* timer;

    /** @brief Used to store the last time a message was sent*/
    simtime_t sendTime;
};

#endif

