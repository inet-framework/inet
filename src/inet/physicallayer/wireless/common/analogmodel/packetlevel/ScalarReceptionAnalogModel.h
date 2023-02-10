//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARRECEPTIONANALOGMODEL_H
#define __INET_SCALARRECEPTIONANALOGMODEL_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API ScalarReceptionAnalogModel : public INarrowbandReceptionAnalogModel
{
  protected:
    const Hz centerFrequency = Hz(NaN);
    const Hz bandwidth = Hz(NaN);
    const W power = W(NaN);

  public:
    ScalarReceptionAnalogModel(Hz centerFrequency, Hz bandwidth, W power) : centerFrequency(centerFrequency), bandwidth(bandwidth), power(power) {}

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const Hz getBandwidth() const override { return bandwidth; }
    virtual const Hz getCenterFrequency() const override { return centerFrequency; }
    virtual const W getPower() const { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

