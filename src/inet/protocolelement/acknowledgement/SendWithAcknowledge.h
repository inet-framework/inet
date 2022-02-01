//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDWITHACKNOWLEDGE_H
#define __INET_SENDWITHACKNOWLEDGE_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SendWithAcknowledge : public PacketFlowBase
{
  protected:
    simtime_t acknowledgeTimeout = -1;

    int sequenceNumber = -1;
    std::map<int, cMessage *> timers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

