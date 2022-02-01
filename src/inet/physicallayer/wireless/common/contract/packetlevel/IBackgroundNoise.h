//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IBACKGROUNDNOISE_H
#define __INET_IBACKGROUNDNOISE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {
namespace physicallayer {

/**
 * This interface models a source which provides background noise over space and time.
 */
class INET_API IBackgroundNoise : public IPrintableObject
{
  public:
    virtual const INoise *computeNoise(const IListening *listening) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

