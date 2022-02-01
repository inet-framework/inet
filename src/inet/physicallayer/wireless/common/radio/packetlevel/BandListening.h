//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BANDLISTENING_H
#define __INET_BANDLISTENING_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ListeningBase.h"

namespace inet {
namespace physicallayer {

using namespace inet::units::values;

class INET_API BandListening : public ListeningBase
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    BandListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const { return centerFrequency; }
    virtual Hz getBandwidth() const { return bandwidth; }
};

} // namespace physicallayer
} // namespace inet

#endif

