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

#include <omnetpp.h>
#include "IPvXAddress.h"

class PingPayload;


/**
 * Generates ping requests and calculates the packet loss and round trip
 * parameters of the replies.
 *
 * See NED file for detailed description of operation.
 */
class INET_API PingApp : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:
    virtual void sendPing();
    virtual void scheduleNextPing(cMessage *timer);
    virtual void sendToICMP(cMessage *payload, const IPvXAddress& destAddr, const IPvXAddress& srcAddr, int hopLimit);
    virtual void processPingResponse(PingPayload *msg);
    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt);

  protected:
    // configuration
    IPvXAddress destAddr;
    IPvXAddress srcAddr;
    int packetSize;
    cPar *intervalp;
    int hopLimit;
    int count;
    simtime_t startTime;
    simtime_t stopTime;
    bool printPing;

    // state
    long sendSeqNo;
    long expectedReplySeqNo;

    // statistics
    cStdDev delayStat;
    cOutVector delayVector;
    cOutVector dropVector;
    long dropCount;
    long outOfOrderArrivalCount;
    long numPongs;
};


