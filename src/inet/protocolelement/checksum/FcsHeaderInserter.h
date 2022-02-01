//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FCSHEADERINSERTER_H
#define __INET_FCSHEADERINSERTER_H

#include "inet/protocolelement/checksum/base/FcsInserterBase.h"
#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

using namespace inet::queueing;

class INET_API FcsHeaderInserter : public FcsInserterBase
{
  protected:
    HeaderPosition headerPosition;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

