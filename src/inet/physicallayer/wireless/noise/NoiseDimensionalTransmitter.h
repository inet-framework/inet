//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NOISEDIMENSIONALTRANSMITTER_H
#define __INET_NOISEDIMENSIONALTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"

namespace inet {

namespace physicallayer {

class INET_API NoiseDimensionalTransmitter : public TransmitterBase, public DimensionalTransmitterBase
{
  protected:
    cPar *durationParameter = nullptr;
    cPar *centerFrequencyParameter = nullptr;
    cPar *bandwidthParameter = nullptr;
    cPar *powerParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

