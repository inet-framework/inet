//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CRCHEADERINSERTER_H
#define __INET_CRCHEADERINSERTER_H

#include "inet/protocolelement/checksum/base/CrcInserterBase.h"
#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

using namespace inet::queueing;

class INET_API CrcHeaderInserter : public CrcInserterBase
{
  protected:
    HeaderPosition headerPosition = HP_UNDEFINED;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

