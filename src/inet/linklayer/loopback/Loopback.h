//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LOOPBACK_H
#define __INET_LOOPBACK_H

#include "inet/linklayer/base/MacProtocolBase.h"

namespace inet {

class NetworkInterface;

/**
 * Loopback interface implementation.
 */
class INET_API Loopback : public MacProtocolBase
{
  protected:
    // statistics
    long numSent = 0;
    long numRcvdOK = 0;

  protected:
    virtual void configureNetworkInterface() override;

  public:
    Loopback() {}
    virtual ~Loopback();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void refreshDisplay() const override;
};

} // namespace inet

#endif

