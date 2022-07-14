//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IPV4DECAPSULATE_H
#define __INET_IPV4DECAPSULATE_H

#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

class INET_API Ipv4Decapsulate : public queueing::PacketPusherBase
{
  protected:
    virtual void pushPacket(Packet *packet, cGate *gate) override;

    void decapsulate(Packet *packet);
};

} // namespace inet

#endif
