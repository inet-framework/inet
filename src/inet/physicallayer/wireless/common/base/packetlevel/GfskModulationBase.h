//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GFSKMODULATIONBASE_H
#define __INET_GFSKMODULATIONBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API GfskModulationBase : public IModulation
{
  protected:
    unsigned int constellationSize;

  public:
    GfskModulationBase(unsigned int constellationSize);

    double calculateBER(double snir, Hz bandwidth, bps bitrate) const override { throw cRuntimeError("Unimplemented!"); }
    double calculateSER(double snir, Hz bandwidth, bps bitrate) const override { throw cRuntimeError("Unimplemented!"); }

    unsigned int getConstellationSize() const { return constellationSize; }
};

} // namespace physicallayer

} // namespace inet

#endif

