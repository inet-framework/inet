//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKDIMENSIONALTRANSMITTER_H
#define __INET_APSKDIMENSIONALTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API ApskDimensionalTransmitter : public DimensionalTransmitterBase, public FlatTransmitterBase
{
  protected:
    void initialize(int stage) override;

  public:
    ApskDimensionalTransmitter();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

