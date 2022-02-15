//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LAYEREDPROTOCOLBASE_H
#define __INET_LAYEREDPROTOCOLBASE_H

#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"

namespace inet {

class INET_API LayeredProtocolBase : public OperationalBase
{
  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleSelfMessage(cMessage *message);

    virtual void handleUpperMessage(cMessage *message);
    virtual void handleLowerMessage(cMessage *message);

    virtual void handleUpperCommand(cMessage *message);
    virtual void handleLowerCommand(cMessage *message);

    virtual void handleUpperPacket(Packet *packet);
    virtual void handleLowerPacket(Packet *packet);

    virtual bool isUpperMessage(cMessage *message) const = 0;
    virtual bool isLowerMessage(cMessage *message) const = 0;
};

} // namespace inet

#endif

