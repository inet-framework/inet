//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARSIGNALANALOGMODEL_H
#define __INET_SCALARSIGNALANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/common/NarrowbandSignalAnalogModel.h"
#include "../../contract/packetlevel/IScalarSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarSignalAnalogModel : public NarrowbandSignalAnalogModel, public virtual IScalarSignalAnalogModel
{
  protected:
    const W power;

  public:
    ScalarSignalAnalogModel(const simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual W getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

