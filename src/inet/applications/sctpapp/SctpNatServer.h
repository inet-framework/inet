//
// Copyright 2007 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SCTPNATSERVER_H
#define __INET_SCTPNATSERVER_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */

typedef struct natInfo
{
    bool multi;
    uint32_t peer1;
    L3Address peer1Address1;
    L3Address peer1Address2;
    uint32_t peer1Assoc;
    uint32_t peer1Port;
    int32_t peer1Gate;
    uint32_t peer2;
    L3Address peer2Address1;
    L3Address peer2Address2;
    uint32_t peer2Assoc;
    uint32_t peer2Port;
    int32_t peer2Gate;
} NatInfo;
typedef std::vector<NatInfo *> NatVector;

class INET_API SctpNatServer : public cSimpleModule, public LifecycleUnsupported
{
  protected:
    int32_t notifications;
    uint32_t assocId;
    SctpSocket *socket;
    bool shutdownReceived;
    int64_t bytesSent;
    int32_t packetsSent;
    int32_t packetsRcvd;
    int32_t numSessions;
    int32_t numRequestsToSend; // requests to send in this session
    bool ordered;
    int32_t outboundStreams;
    int32_t inboundStreams;
    int32_t lastStream;

    NatVector& natVector = SIMULATION_SHARED_VARIABLE(natVector);

    int32_t ssn;

  public:
    struct pathStatus {
        bool active;
        bool primaryPath;
        L3Address pid;
    };

    void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void handleTimer(cMessage *msg);
    void generateAndSend();
    void sendInfo(NatInfo *info);
    void printNatVector();
};

} // namespace inet

#endif

