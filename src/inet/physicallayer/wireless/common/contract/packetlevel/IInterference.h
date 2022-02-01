//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IINTERFERENCE_H
#define __INET_IINTERFERENCE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the interference related to a reception.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IInterference : public IPrintableObject
{
  public:
    virtual const INoise *getBackgroundNoise() const = 0;

    virtual const std::vector<const IReception *> *getInterferingReceptions() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

