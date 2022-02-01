//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ETHERAPPSERVER_H
#define __INET_ETHERAPPSERVER_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

#define MAX_REPLY_CHUNK_SIZE    1497

/**
 * Server-side process EtherAppClient.
 */
class INET_API EtherAppServer : public ApplicationBase, public Ieee8022LlcSocket::ICallback
{
  protected:
    int localSap = 0;

    Ieee8022LlcSocket llcSocket;

    // statistics
    long packetsSent = 0;
    long packetsReceived = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    void registerDsap(int dsap);
    void sendPacket(Packet *datapacket, int interfaceId, const MacAddress& destAddr, int destSap);
    virtual void socketDataArrived(Ieee8022LlcSocket *, Packet *msg) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;
};

} // namespace inet

#endif

