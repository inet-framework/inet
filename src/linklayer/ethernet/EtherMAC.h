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

#include "INETDefs.h"

#include "EtherMACBase.h"


class EtherJam;
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
    virtual void initializeFlags();
    virtual void initializeStatistics();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:
    // states
    int  backoffs;                     // value of backoff for exponential back-off algorithm
    long currentSendPkTreeID;

    // other variables
    EtherTraffic *frameBeingReceived;
    cMessage *endRxMsg, *endBackoffMsg, *endJammingMsg;

    // statistics
    simtime_t totalCollisionTime;      // total duration of collisions on channel
    simtime_t totalSuccessfulRxTxTime; // total duration of successful transmissions on channel
    simtime_t channelBusySince;        // needed for computing totalCollisionTime/totalSuccessfulRxTxTime
    unsigned long numCollisions;       // collisions (NOT number of collided frames!) sensed
    unsigned long numBackoffs;         // number of retransmissions
    unsigned int  framesSentInBurst;   // Number of frames send out in current frame burst
    long bytesSentInBurst;             // Number of bytes transmitted in current frame burst

    struct PkIdRxTime
    {
        long packetTreeId;             // tree ID of packet being received.
        simtime_t endTime;             // end of reception
        PkIdRxTime(long id, simtime_t time) {packetTreeId=id; endTime = time;}
    };
    typedef std::list<PkIdRxTime> EndRxTimeList;
    EndRxTimeList endRxTimeList;       // list of incoming packets, ordered by endTime
    int numConcurrentTransmissions;    // number of colliding frames -- we must receive this many jams (caches endRxTimeList.size())

    static simsignal_t collisionSignal;
    static simsignal_t backoffSignal;

  protected:
    // event handlers
    virtual void handleSelfMessage(cMessage *msg);
    virtual void handleEndIFGPeriod();
    virtual void handleEndPausePeriod();
    virtual void handleEndTxPeriod();
    virtual void handleEndRxPeriod();
    virtual void handleEndBackoffPeriod();
    virtual void handleEndJammingPeriod();
    virtual void handleRetransmission();

    // helpers
    virtual void calculateParameters(bool errorWhenAsymmetric);
    virtual void processFrameFromUpperLayer(EtherFrame *msg);
    virtual void processMsgFromNetwork(EtherTraffic *msg);
    virtual void processMessageWhenNotConnected(cMessage *msg);
    virtual void processMessageWhenDisabled(cMessage *msg);
    virtual void scheduleEndIFGPeriod();
    virtual void scheduleEndTxPeriod(EtherFrame *);
    virtual void scheduleEndRxPeriod(EtherTraffic *);
    virtual void scheduleEndPausePeriod(int pauseUnits);
    virtual bool checkAndScheduleEndPausePeriod();
    virtual void beginSendFrames();
    virtual void sendJamSignal();
    virtual void startFrameTransmission();
    virtual void frameReceptionComplete();
    virtual void processReceivedDataFrame(EtherFrame *frame);
    virtual void processReceivedJam(EtherJam *jam);
    virtual void processPauseCommand(int pauseUnits);
    virtual void processConnectionChanged();
    virtual void processEndReception(simtime_t endRxTime);
    virtual void processEndReceptionAtReconnectState(long id, simtime_t endRxTime);
    virtual void processDetectedCollision();

    virtual void printState();
};

#endif

