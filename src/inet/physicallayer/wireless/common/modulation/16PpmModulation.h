//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_16PPMMODULATION_H
#define __INET_16PPMMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PpmModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API _16PpmModulation : public PpmModulationBase
{
  public:
    static const _16PpmModulation singleton;

  public:
    _16PpmModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "16PpmModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

