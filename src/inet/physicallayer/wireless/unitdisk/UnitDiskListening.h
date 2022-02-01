//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKLISTENING_H
#define __INET_UNITDISKLISTENING_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ListeningBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model doesn't specify any listening properties.
 */
class INET_API UnitDiskListening : public ListeningBase
{
  public:
    UnitDiskListening(const IRadio *radio, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

