//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKRECEPTIONANALOGMODEL_H
#define __INET_UNITDISKRECEPTIONANALOGMODEL_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/analogmodel/bitlevel/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API UnitDiskReceptionAnalogModel : public SignalAnalogModel, public IReceptionAnalogModel
{
  public:
    enum Power {
        POWER_UNDETECTABLE,
        POWER_DETECTABLE,
        POWER_INTERFERING,
        POWER_RECEIVABLE
    };

  protected:
    const Power power;

  public:
    UnitDiskReceptionAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, const Power power) :
        SignalAnalogModel(preambleDuration, headerDuration, dataDuration),
        power(power) {}

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Power getPower() const { return power; }
//    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { throw cRuntimeError("Invalid operation"); }
};

} // namespace physicallayer

} // namespace inet

#endif

