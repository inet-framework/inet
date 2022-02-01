//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEPTIONRESULT_H
#define __INET_RECEPTIONRESULT_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionResult.h"

namespace inet {
namespace physicallayer {

class INET_API ReceptionResult : public IReceptionResult, public cObject
{
  protected:
    const IReception *reception;
    const std::vector<const IReceptionDecision *> *decisions;
    const Packet *packet;

  public:
    ReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const Packet *packet);
    virtual ~ReceptionResult();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReception *getReception() const override { return reception; }
    virtual const std::vector<const IReceptionDecision *> *getDecisions() const override { return decisions; }

    virtual const Packet *getPacket() const override;
};

} // namespace physicallayer
} // namespace inet

#endif

