//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARRECEPTION_H
#define __INET_SCALARRECEPTION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceptionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarReception : public FlatReceptionBase, public virtual IScalarSignal
{
  protected:
    const W power;

  public:
    ScalarReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth, W power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual W getPower() const override { return power; }
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const override { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

