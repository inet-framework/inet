//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022LLCINSERTER_H
#define __INET_IEEE8022LLCINSERTER_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8022LlcInserter : public PacketFlowBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    static const Protocol *getProtocol(const Ptr<const Ieee8022LlcHeader>& header);
};

} // namespace inet

#endif

