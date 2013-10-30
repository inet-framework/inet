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

#ifndef __INET_ETHERAPPCLI_H
#define __INET_ETHERAPPCLI_H

#include "INETDefs.h"

#include "MACAddress.h"
#include "NodeStatus.h"
#include "ILifecycle.h"


/**
 * Simple traffic generator for the Ethernet model.
 */
class INET_API EtherAppCli : public InetSimpleModule, public ILifecycle
{
  protected:
    enum Kinds {START=100, NEXT};

    // send parameters
    long seqNum;
    cPar *reqLength;
    cPar *respLength;
    cPar *sendInterval;

    int localSAP;
    int remoteSAP;
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
    EtherAppCli();
    virtual ~EtherAppCli();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual bool isNodeUp();
    virtual bool isGenerator();
    virtual void scheduleNextPacket(bool start);
    virtual void cancelNextPacket();

    virtual MACAddress resolveDestMACAddress();

    virtual void sendPacket();
    virtual void receivePacket(cPacket *msg);
    virtual void registerDSAP(int dsap);
};

#endif
