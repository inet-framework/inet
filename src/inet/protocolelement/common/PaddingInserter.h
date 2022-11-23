//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PADDINGINSERTER_H
#define __INET_PADDINGINSERTER_H

#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"
#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

class INET_API PaddingInserter : public ProtocolHeaderInserterBase
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

