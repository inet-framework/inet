//
// Copyright (C) 2020 OpenSimLtd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EXTETHERNETTAPDEVICE_H
#define __INET_EXTETHERNETTAPDEVICE_H

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/scheduler/RealTimeScheduler.h"

namespace inet {

class INET_API ExtEthernetTapDevice : public cSimpleModule, public RealTimeScheduler::ICallback
{
  protected:
    // parameters
    std::string device;
    const char *packetNameFormat = nullptr;
    RealTimeScheduler *rtScheduler = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;

    // state
    PacketPrinter packetPrinter;
    int fd = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void finish() override;

    virtual void openTap(std::string dev);
    virtual void closeTap();

  public:
    virtual ~ExtEthernetTapDevice();

    virtual bool notify(int fd) override;
};

} // namespace inet

#endif

