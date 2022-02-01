//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PADDINGINSERTER_H
#define __INET_PADDINGINSERTER_H

#include "inet/protocolelement/common/HeaderPosition.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API PaddingInserter : public PacketFlowBase
{
  protected:
    b minLength = b(-1);
    b roundingLength = b(-1);
    HeaderPosition insertionPosition;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

