//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETFCSINSERTER_H
#define __INET_ETHERNETFCSINSERTER_H

#include "inet/protocolelement/checksum/base/FcsInserterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetFcsInserter : public FcsInserterBase
{
  protected:
    virtual uint32_t computeFcs(const Packet *packet, FcsMode fcsMode) const override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

