//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CHECKSUMHEADERCHECKER_H
#define __INET_CHECKSUMHEADERCHECKER_H

#include "inet/protocolelement/checksum/base/ChecksumCheckerBase.h"
#include "inet/protocolelement/common/HeaderPosition.h"

namespace inet {

using namespace inet::queueing;

class INET_API ChecksumHeaderChecker : public ChecksumCheckerBase
{
  protected:
    HeaderPosition headerPosition;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool matchesPacket(const Packet *packet) const override;
};

} // namespace inet

#endif

