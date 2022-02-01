//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDESERIALIZER_H
#define __INET_PACKETDESERIALIZER_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketDeserializer : public PacketFlowBase
{
  protected:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

