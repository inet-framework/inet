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

#ifndef __INET_SCTPCLIENT_H
#define __INET_SCTPCLIENT_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"

namespace inet {

namespace sctp {

class SctpAssociation;

} // namespace sctp

/**
 * Implements the SctpClient simple module. See the NED file for more info.
 */
class INET_API SctpClient : public cSimpleModule, public SctpSocket::ICallback, public LifecycleUnsupported
{
  protected:
    struct PathStatus
    {
        L3Address pid;
        bool active;
        bool primaryPath;
    };
    typedef std::map<L3Address, PathStatus> SctpPathStatus;

    // parameters: see the corresponding NED variables
    std::map<unsigned int, unsigned int> streamRequestLengthMap;
    std::map<unsigned int, unsigned int> streamRequestRatioMap;
    std::map<unsigned int, unsigned int> streamRequestRatioSendMap;
    int queueSize;
    unsigned int outStreams;
    unsigned int inStreams;
    bool echo;
    bool ordered;
    bool finishEndsSimulation;

    // state
    SctpSocket socket;
    SctpPathStatus sctpPathStatus;
    cMessage *timeMsg;
    cMessage *stopTimer;
    cMessage *primaryChangeTimer;
    int64 bufferSize;
    bool timer;
    bool sendAllowed;
    const char *stateNameStr;

    // statistics
    unsigned long int packetsSent;
    unsigned long int packetsRcvd;
    unsigned long int bytesSent;
    unsigned long int echoedBytesSent;
    unsigned long int bytesRcvd;
    unsigned long int numRequestsToSend;    // requests to send in this session
    unsigned long int numPacketsToReceive;
    int numSessions;
    int numBroken;
    int chunksAbandoned;
    static simsignal_t echoedPkSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    void connect();
    void close();
    void handleTimer(cMessage *msg);

    /* SctpSocket::ICallback callback methods */
    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override { throw cRuntimeError("Model error, this module doesn't use any listener SCTP sockets"); }
    void socketEstablished(SctpSocket *socket, unsigned long int buffer) override;    // TODO: needs a better name
    void socketDataArrived(SctpSocket *socket, Packet *msg, bool urgent) override;    // TODO: needs a better name
    void socketDataNotificationArrived(SctpSocket *socket, Message *msg) override;
    void socketPeerClosed(SctpSocket *socket) override;
    void socketClosed(SctpSocket *socket) override;
    void socketFailure(SctpSocket *socket, int code) override;
    void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) override;

    void setPrimaryPath(const char *addr);
    void sendRequestArrived(SctpSocket *socket) override;
    void sendQueueRequest();
    void shutdownReceivedArrived(SctpSocket *socket) override;
    void sendqueueAbatedArrived(SctpSocket *socket, unsigned long int buffer) override;
    void msgAbandonedArrived(SctpSocket *socket) override;
    void sendStreamResetNotification();
    void sendRequest(bool last = true);

  public:
    SctpClient();
    virtual ~SctpClient();
};

} // namespace inet

#endif // ifndef __INET_SCTPCLIENT_H

