//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISOTROPICDIMENSIONALBACKGROUNDNOISE_H
#define __INET_ISOTROPICDIMENSIONALBACKGROUNDNOISE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IBackgroundNoise.h"

namespace inet {

namespace physicallayer {

class INET_API IsotropicDimensionalBackgroundNoise : public cModule, public IBackgroundNoise
{
  protected:
    WpHz powerSpectralDensity = WpHz(NaN);
    W power = W(NaN);
    mutable Hz bandwidth = Hz(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const INoise *computeNoise(const IListening *listening) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

