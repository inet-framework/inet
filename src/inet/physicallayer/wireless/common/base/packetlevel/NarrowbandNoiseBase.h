//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDNOISEBASE_H
#define __INET_NARROWBANDNOISEBASE_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NoiseBase.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

// REFACTOR TODO
class INET_API NarrowbandNoiseBase : public NoiseBase
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandNoiseBase(simtime_t startTime, simtime_t endTime, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const { return centerFrequency; }
    virtual Hz getBandwidth() const { return bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif

