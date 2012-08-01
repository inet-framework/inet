//
// Copyright (C) 2012 OpenSim Ltd.
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

class PingPayload;

// how many ping request's send time is stored
#define PINGTEST_HISTORY_SIZE 100

/**
 * Generates ping requests and calculates the packet loss and round trip
 * parameters of the replies.
 *
 * See NED file for detailed description of operation.
 */
class INET_API PingTestApp : public cSimpleModule
{
  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 4; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:
    virtual std::vector<IPvXAddress> getAllAddresses();
    virtual void sendPing();
    virtual void scheduleNextPing(cMessage *timer);
    virtual void sendToICMP(cMessage *payload, const IPvXAddress& destAddr, const IPvXAddress& srcAddr, int hopLimit);
    virtual void processPingResponse(PingPayload *msg);
    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);

  protected:
    // configuration
    IPvXAddress destAddr;
    IPvXAddress srcAddr;
    std::vector<IPvXAddress> destAddresses;
    int packetSize;
    cPar *sendIntervalp;
    int hopLimit;
    int count;
    simtime_t startTime;
    simtime_t stopTime;
    bool printPing;
    bool continuous;

    // state
    long sendSeqNo;
    long expectedReplySeqNo;
    simtime_t sendTimeHistory[PINGTEST_HISTORY_SIZE];

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


