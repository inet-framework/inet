//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPSERVER_H
#define __INET_SCTPSERVER_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

/**
 * Implements the SctpServer simple module. See the NED file for more info.
 */
class INET_API SctpServer : public cSimpleModule, public LifecycleUnsupported
{
  protected:
    struct ServerAssocStat
    {
        simtime_t start;
        simtime_t stop;
        simtime_t lifeTime;
        unsigned long int rcvdBytes;
        unsigned long int sentPackets;
        unsigned long int rcvdPackets;
        bool abortSent;
        bool peerClosed;
    };
    typedef std::map<int, ServerAssocStat> ServerAssocStatMap;
    typedef std::map<int, cOutVector *> BytesPerAssoc;
    typedef std::map<int, cOutVector *> EndToEndDelay;

    // parameters
    int inboundStreams;
    int outboundStreams;
    int queueSize;
    double delay;
    double delayFirstRead;
    bool finishEndsSimulation;
    bool echo;
    bool ordered;

    // state
    SctpSocket *socket;
    cMessage *timeoutMsg;
    cMessage *delayTimer;
    cMessage *delayFirstReadTimer;
    int lastStream;
    int assocId;
    bool readInt;
    bool schedule;
    bool firstData;
    bool shutdownReceived;
    bool abortSent;
    EndToEndDelay endToEndDelay;

    // statistics
    int numSessions;
    int count;
    int notificationsReceived;
    unsigned long int bytesSent;
    unsigned long int packetsSent;
    unsigned long int packetsRcvd;
    unsigned long int numRequestsToSend;    // requests to send in this session
    BytesPerAssoc bytesPerAssoc;
    ServerAssocStatMap serverAssocStatMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    void handleTimer(cMessage *msg);
    void sendOrSchedule(cMessage *msg);
    void sendOrSchedule(Message *msg);
    void sendOrSchedule(Packet *pkt);

    Message *makeAbortNotification(SctpCommandReq *msg);
    Message *makeReceiveRequest(cMessage *msg);
    Message *makeDefaultReceive();
    void generateAndSend();

  public:
    virtual ~SctpServer();
    SctpServer();
};

} // namespace inet

#endif // ifndef __INET_SCTPSERVER_H

