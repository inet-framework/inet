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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

class PingPayload;

#define PING_HISTORY_SIZE    10 // how many ping request's send time is stored

/**
 * Generates ping requests and calculates the packet loss and round trip
 * parameters of the replies.
 *
 * See NED file for detailed description of operation.
 */
class INET_API PingApp : public cSimpleModule, public ILifecycle
{
  protected:
    // parameters: for more details, see the corresponding NED parameters' documentation
    L3Address destAddr;
    L3Address srcAddr;
    int packetSize;
    cPar *sendIntervalPar;
    int hopLimit;
    int count;
    simtime_t startTime;
    simtime_t stopTime;
    bool printPing;

    // state
    int pid;    // to determine which hosts are associated with the responses
    cMessage *timer;    // to schedule the next Ping request
    NodeStatus *nodeStatus;    // lifecycle
    simtime_t lastStart;    // the last time when the app was started (lifecycle)
    long sendSeqNo;    // to match the response with the request that caused the response
    long expectedReplySeqNo;
    simtime_t sendTimeHistory[PING_HISTORY_SIZE];    // times of when the requests were sent

    // statistics
    cStdDev rttStat;
    static simsignal_t rttSignal;
    static simsignal_t numLostSignal;
    static simsignal_t outOfOrderArrivalsSignal;
    static simsignal_t pingTxSeqSignal;
    static simsignal_t pingRxSeqSignal;
    long sentCount;    // number of sent Ping requests
    long lossCount;    // number of lost requests
    long outOfOrderArrivalCount;    // number of responses which arrived too late
    long numPongs;    // number of received Ping requests

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void startSendingPingRequests();
    virtual void stopSendingPingRequests();
    virtual void scheduleNextPingRequest(simtime_t previous);
    virtual void cancelNextPingRequest();
    virtual bool isNodeUp();
    virtual bool isEnabled();
    virtual void sendPingRequest();
    virtual void sendToICMP(PingPayload *payload, const L3Address& destAddr, const L3Address& srcAddr, int hopLimit);
    virtual void processPingResponse(PingPayload *msg);
    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  public:
    PingApp();
    virtual ~PingApp();
};

} // namespace inet

