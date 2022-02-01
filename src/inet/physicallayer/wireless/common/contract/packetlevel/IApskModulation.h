//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IAPSKMODULATION_H
#define __INET_IAPSKMODULATION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API IApskModulation : public IModulation
{
  public:
    virtual unsigned int getCodeWordSize() const = 0;
    virtual unsigned int getConstellationSize() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

