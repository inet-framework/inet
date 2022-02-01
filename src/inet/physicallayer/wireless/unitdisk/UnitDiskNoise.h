//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKNOISE_H
#define __INET_UNITDISKNOISE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/NoiseBase.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskNoise : public NoiseBase
{
  protected:
    bool isInterfering_;

  public:
    UnitDiskNoise(simtime_t startTime, simtime_t endTime, bool isInterfering);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual bool isInterfering() const { return isInterfering_; }
};

} // namespace physicallayer

} // namespace inet

#endif

