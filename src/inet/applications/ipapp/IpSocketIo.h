//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPSOCKETIO_H
#define __INET_IPSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/networklayer/contract/ipv4/Ipv4Socket.h"

namespace inet {

class INET_API IpSocketIo : public ApplicationBase, public Ipv4Socket::ICallback
{
  protected:
    const Protocol *protocol = nullptr;
    bool dontFragment = false;
    Ipv4Socket socket;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void setSocketOptions();

    virtual void socketDataArrived(Ipv4Socket *socket, Packet *packet) override;
    virtual void socketClosed(Ipv4Socket *socket) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

