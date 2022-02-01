//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INSTANTSERVER_H
#define __INET_INSTANTSERVER_H

#include "inet/queueing/base/PacketServerBase.h"

namespace inet {
namespace queueing {

class INET_API InstantServer : public PacketServerBase
{
  protected:
    bool isProcessing = false;

  protected:
    virtual bool canProcessPacket();
    virtual void processPacket();
    virtual void processPackets();

  public:
    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

