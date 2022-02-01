//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALBACKGROUNDNOISE_H
#define __INET_DIMENSIONALBACKGROUNDNOISE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DimensionalTransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IBackgroundNoise.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalBackgroundNoise : public DimensionalTransmitterBase, public cModule, public IBackgroundNoise
{
  protected:
    W power;

  protected:
    virtual void initialize(int stage) override;

  public:
    DimensionalBackgroundNoise();

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const INoise *computeNoise(const IListening *listening) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

