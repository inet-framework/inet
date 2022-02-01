//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISNIR_H
#define __INET_ISNIR_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/INoise.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"

namespace inet {

namespace physicallayer {

class INET_API ISnir : public IPrintableObject
{
  public:
    virtual const IReception *getReception() const = 0;

    virtual const INoise *getNoise() const = 0;

    virtual double getMin() const = 0;
    virtual double getMax() const = 0;
    virtual double getMean() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

