//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_4GFSKMODULATION_H
#define __INET_4GFSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/GfskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API _4GfskModulation : public GfskModulationBase
{
  public:
    static const _4GfskModulation singleton;

  public:
    _4GfskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "4GfskModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

