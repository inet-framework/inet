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

#ifndef __INET_ETHERTRAFGEN_H
#define __INET_ETHERTRAFGEN_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "NodeStatus.h"
#include "ILifecycle.h"


/**
 * Simple traffic generator for the Ethernet model.
 */
class INET_API EtherTrafGen : public cSimpleModule, public ILifecycle
{
  protected:
    enum Kinds {START=100, NEXT};

    long seqNum;

    // send parameters
    cPar *sendInterval;
    cPar *numPacketsPerBurst;
    cPar *packetLength;
    int etherType;
    MACAddress destMACAddress;
    NodeStatus *nodeStatus;

    // self messages
    cMessage *timerMsg;
    simtime_t startTime;
    simtime_t stopTime;

    // receive statistics
    long packetsSent;
    long packetsReceived;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  public:
    EtherTrafGen();
    virtual ~EtherTrafGen();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 4; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual bool isNodeUp();
    virtual bool isGenerator();
    virtual void scheduleNextPacket(simtime_t previous);
    virtual void cancelNextPacket();

    virtual MACAddress resolveDestMACAddress();

    virtual void sendBurstPackets();
    virtual void receivePacket(cPacket *msg);
};

#endif
