//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_4PPMMODULATION_H
#define __INET_4PPMMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PpmModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API _4PpmModulation : public PpmModulationBase
{
  public:
    static const _4PpmModulation singleton;

  public:
    _4PpmModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "4PpmModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

