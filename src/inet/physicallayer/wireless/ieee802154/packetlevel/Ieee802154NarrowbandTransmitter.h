//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154NARROWBANDSCALARTRANSMITTER_H
#define __INET_IEEE802154NARROWBANDSCALARTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee802154NarrowbandScalarTransmitter : public FlatTransmitterBase
{
  public:
    Ieee802154NarrowbandScalarTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

