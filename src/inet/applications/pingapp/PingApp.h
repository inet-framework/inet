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

#ifndef __INET_PINGAPP_H
#define __INET_PINGAPP_H

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/INetworkSocket.h"
#include "inet/transportlayer/common/CrcMode_m.h"

namespace inet {

// how many ping request's send time is stored
#define PING_HISTORY_SIZE    100

/**
 * Generates ping requests and calculates the packet loss and round trip
 * parameters of the replies.
 *
 * See NED file for detailed description of operation.
 */
class INET_API PingApp : public ApplicationBase, public INetworkSocket::ICallback
{
  protected:
    // parameters: for more details, see the corresponding NED parameters' documentation
    L3Address destAddr;
    L3Address srcAddr;
    std::vector<L3Address> destAddresses;
    int packetSize = 0;
    cPar *sendIntervalPar = nullptr;
    cPar *sleepDurationPar = nullptr;
    int hopLimit = 0;
    int count = 0;
    int destAddrIdx = -1;
    simtime_t startTime;
    simtime_t stopTime;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    bool printPing = false;
    bool continuous = false;

    // state
    SocketMap socketMap;
    INetworkSocket *currentSocket = nullptr;   // current socket stored in socketMap, too
    int pid = 0;    // to determine which hosts are associated with the responses
    cMessage *timer = nullptr;    // to schedule the next Ping request
    NodeStatus *nodeStatus = nullptr;    // lifecycle
    simtime_t lastStart;    // the last time when the app was started (lifecycle)
    long sendSeqNo = 0;    // to match the response with the request that caused the response
    long expectedReplySeqNo = 0;
    simtime_t sendTimeHistory[PING_HISTORY_SIZE];    // times of when the requests were sent
    bool pongReceived[PING_HISTORY_SIZE];

    static const std::map<const Protocol *, const Protocol *> l3Echo;

    // statistics
    cStdDev rttStat;
    static simsignal_t rttSignal;
    static simsignal_t numLostSignal;
    static simsignal_t numOutOfOrderArrivalsSignal;
    static simsignal_t pingTxSeqSignal;
    static simsignal_t pingRxSeqSignal;
    long sentCount = 0;    // number of sent Ping requests
    long lossCount = 0;    // number of lost requests
    long outOfOrderArrivalCount = 0;    // number of responses which arrived too late
    long numPongs = 0;    // number of received Ping requests
    long numDuplicatedPongs = 0;    // number of duplicated Ping responses

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleSelfMessage(cMessage *msg);
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void parseDestAddressesPar();
    virtual void startSendingPingRequests();
    virtual void scheduleNextPingRequest(simtime_t previous, bool withSleep);
    virtual void cancelNextPingRequest();
    virtual bool isEnabled();
    virtual std::vector<L3Address> getAllAddresses();
    virtual void sendPingRequest();
    virtual void processPingResponse(int identifier, int seqNumber, Packet *packet);
    virtual void countPingResponse(int bytes, long seqNo, simtime_t rtt, bool isDup);

    // Lifecycle methods
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    //INetworkSocket::ICallback:
    virtual void socketDataArrived(INetworkSocket *socket, Packet *packet) override;
    virtual void socketClosed(INetworkSocket *socket) override;
  public:
    PingApp();
    virtual ~PingApp();
    int getPid() const { return pid; }
};

} // namespace inet

#endif
