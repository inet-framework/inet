//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ECHOPROTOCOL_H
#define __INET_ECHOPROTOCOL_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/networklayer/common/L3Address.h"

namespace inet {

/**
 * TODO
 */
class INET_API EchoProtocol : public cSimpleModule
{
  protected:
    virtual void processPacket(Packet *packet);
    virtual void processEchoRequest(Packet *packet);
    virtual void processEchoReply(Packet *packet);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace inet

#endif

