//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MPDUGEN_H
#define __INET_MPDUGEN_H

#include "inet/common/packet/Packet.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
/*
 * A very simple MPDU generator class.
 */
class MpduGen : public ApplicationBase, UdpSocket::ICallback
{
protected:
    int localPort = -1, destPort = -1;
    UdpSocket socket;

    cMessage *selfMsg = nullptr;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void sendPackets();
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *pk) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    MpduGen() {}
    ~MpduGen() { cancelAndDelete(selfMsg); }
};

}

#endif
