//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154NARROWBANDDIMENSIONALTRANSMITTER_H
#define __INET_IEEE802154NARROWBANDDIMENSIONALTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee802154NarrowbandDimensionalTransmitter : public FlatTransmitterBase, public DimensionalTransmitterBase
{
  public:
    Ieee802154NarrowbandDimensionalTransmitter();

    virtual void initialize(int stage) override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

