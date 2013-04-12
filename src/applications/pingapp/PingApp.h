//
// Copyright (C) 2001, 2003, 2004 Johnny Lai, Monash University, Melbourne, Australia
// Copyright (C) 2005 Andras Varga
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

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "ILifecycle.h"

class PingPayload;

// how many ping request's send time is stored
#define PING_HISTORY_SIZE 10

/**
 * Generates ping requests and calculates the packet loss and round trip
 * parameters of the replies.
 *
 * See NED file for detailed description of operation.
 */
class INET_API PingApp : public cSimpleModule, public ILifecycle
{
  public:
    PingApp();
    virtual ~PingApp();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:
    virtual bool isRunning() { return timer->isScheduled(); }
    virtual void sendPing();
    virtual void scheduleNextPing();
    virtual void sendToICMP(cMessage *payload, const IPvXAddress& destAddr, const IPvXAddress& srcAddr, int hopLimit);
    virtual void processPingResponse(PingPayload *msg);
    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);

  protected:
    // configuration
    IPvXAddress destAddr;
    IPvXAddress srcAddr;
    int packetSize;
    cPar *sendIntervalp;
    int hopLimit;
    int count;
    simtime_t startTime;
    simtime_t stopTime;
    bool printPing;

    // state
    cMessage *timer;
    long sendSeqNo;
    long expectedReplySeqNo;
    simtime_t sendTimeHistory[PING_HISTORY_SIZE];

    // statistics
    cStdDev rttStat;
    static simsignal_t rttSignal;
    static simsignal_t numLostSignal;
    static simsignal_t outOfOrderArrivalsSignal;
    static simsignal_t pingTxSeqSignal;
    static simsignal_t pingRxSeqSignal;
    long lossCount;
    long outOfOrderArrivalCount;
    long numPongs;
};


