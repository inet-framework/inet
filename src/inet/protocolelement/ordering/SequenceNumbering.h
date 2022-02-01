//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SEQUENCENUMBERING_H
#define __INET_SEQUENCENUMBERING_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API SequenceNumbering : public PacketFlowBase
{
  protected:
    int sequenceNumber = 0;

  protected:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

