//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDRECEPTIONBASE_H
#define __INET_NARROWBANDRECEPTIONBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandReceptionBase : public ReceptionBase, public virtual INarrowbandSignal
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const override { return centerFrequency; }
    virtual Hz getBandwidth() const override { return bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif

