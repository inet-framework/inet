/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __INET_ETHERMAC_H
#define __INET_ETHERMAC_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "INETDefs.h"
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "EtherMACBase.h"

// Length of autoconfig period: should be larger than delays
#define AUTOCONFIG_PERIOD  0.001  /* well more than 4096 bit times at 10Mb */

class IPassiveQueue;

/**
 * Ethernet MAC module.
 */
class INET_API EtherMAC : public EtherMACBase
{
  public:
    EtherMAC();
    virtual ~EtherMAC();

  protected:
    virtual void initialize();
    virtual void initializeTxrate();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:
    // parameters for autoconfig
    bool autoconfigInProgress; // true if autoconfig is currently ongoing
    double lowestTxrateSuggested;
    bool duplexVetoed;

    // states
    int  backoffs;          // Value of backoff for exponential back-off algorithm
    int  numConcurrentTransmissions; // number of colliding frames -- we must receive this many jams

    // other variables
    EtherFrame *frameBeingReceived;
    cMessage *endRxMsg, *endBackoffMsg, *endJammingMsg;

    // statistics
    simtime_t totalCollisionTime;      // total duration of collisions on channel
    simtime_t totalSuccessfulRxTxTime; // total duration of successful transmissions on channel
    simtime_t channelBusySince;        // needed for computing totalCollisionTime/totalSuccessfulRxTxTime
    unsigned long numCollisions;       // collisions (NOT number of collided frames!) sensed
    unsigned long numBackoffs;         // number of retransmissions
    cOutVector numCollisionsVector;
    cOutVector numBackoffsVector;

    // event handlers
    virtual void processFrameFromUpperLayer(EtherFrame *msg);
    virtual void processMsgFromNetwork(cPacket *msg);
    virtual void handleEndIFGPeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndRxPeriod();
    virtual void handleEndBackoffPeriod();
    virtual void handleEndJammingPeriod();

    // setup, autoconfig
    virtual void startAutoconfig();
    virtual void handleAutoconfigMessage(cMessage *msg);
    virtual void printState();

    // helpers
    virtual void scheduleEndRxPeriod(cPacket *);
    virtual void sendJamSignal();
    virtual void handleRetransmission();
    virtual void startFrameTransmission();

    // notifications
    virtual void updateHasSubcribers();
};

#endif


