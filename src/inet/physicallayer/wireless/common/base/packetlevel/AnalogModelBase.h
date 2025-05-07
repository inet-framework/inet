//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ANALOGMODELBASE_H
#define __INET_ANALOGMODELBASE_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IMediumAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntennaGain.h"

namespace inet {

namespace physicallayer {

class INET_API AnalogModelBase : public Module, public virtual IMediumAnalogModel
{
  protected:
    virtual double computeAntennaGain(const IAntennaGain *antenna, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation) const;
};

} // namespace physicallayer

} // namespace inet

#endif

