//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMSPLITTER_H
#define __INET_STREAMSPLITTER_H

#include "inet/queueing/base/PacketDuplicatorBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamSplitter : public PacketDuplicatorBase
{
  protected:
    cValueMap *mapping = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual int getNumPacketDuplicates(Packet *packet) override;
};

} // namespace inet

#endif

