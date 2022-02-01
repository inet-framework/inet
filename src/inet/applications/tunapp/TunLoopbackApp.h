//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TUNLOOPBACKAPP_H
#define __INET_TUNLOOPBACKAPP_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/linklayer/tun/TunSocket.h"

namespace inet {

class INET_API TunLoopbackApp : public cSimpleModule, public LifecycleUnsupported
{
  protected:
    const char *tunInterface = nullptr;

    unsigned int packetsSent = 0;
    unsigned int packetsReceived = 0;

    TunSocket tunSocket;

  protected:
    void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void handleMessage(cMessage *msg) override;
    void finish() override;
};

} // namespace inet

# endif

