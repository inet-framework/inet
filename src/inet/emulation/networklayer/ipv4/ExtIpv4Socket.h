//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EXTIPV4SOCKET_H
#define __INET_EXTIPV4SOCKET_H

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/scheduler/RealTimeScheduler.h"

namespace inet {

class INET_API ExtIpv4Socket : public cSimpleModule, public RealTimeScheduler::ICallback
{
  protected:
    // parameters
    const char *packetNameFormat = nullptr;
    RealTimeScheduler *rtScheduler = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;

    // state
    PacketPrinter packetPrinter;
    int fd = INVALID_SOCKET;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void openSocket();
    virtual void closeSocket();

  public:
    virtual ~ExtIpv4Socket();

    virtual bool notify(int fd) override;
};

} // namespace inet

#endif

