//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETSOCKETIO_H
#define __INET_ETHERNETSOCKETIO_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/Protocol.h"
#include "inet/linklayer/ethernet/common/EthernetSocket.h"

namespace inet {

class INET_API EthernetSocketIo : public ApplicationBase, public EthernetSocket::ICallback
{
  protected:
    NetworkInterface *networkInterface = nullptr;
    const Protocol *protocol = nullptr;
    MacAddress remoteAddress;
    MacAddress localAddress;
    EthernetSocket socket;
    int numSent = 0;
    int numReceived = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    void setSocketOptions();

    virtual void socketDataArrived(EthernetSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(EthernetSocket *socket, Indication *indication) override;
    virtual void socketClosed(EthernetSocket *socket) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
};

} // namespace inet

#endif

