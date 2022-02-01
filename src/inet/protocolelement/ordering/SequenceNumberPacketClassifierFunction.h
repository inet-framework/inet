//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SEQUENCENUMBERPACKETCLASSIFIERFUNCTION_H
#define __INET_SEQUENCENUMBERPACKETCLASSIFIERFUNCTION_H

#include "inet/queueing/contract/IPacketClassifierFunction.h"

namespace inet {

using namespace inet::queueing;

class INET_API SequenceNumberPacketClassifierFunction : public cObject, public IPacketClassifierFunction
{
  protected:
    virtual int classifyPacket(Packet *packet) const override;
};

} // namespace inet

#endif

