//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8022SNAPINSERTER_H
#define __INET_IEEE8022SNAPINSERTER_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"
#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"

namespace inet {

class INET_API Ieee8022SnapInserter : public ProtocolHeaderInserterBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    static const Protocol *getProtocol(const Ptr<const Ieee8022SnapHeader>& header);
};

} // namespace inet

#endif

