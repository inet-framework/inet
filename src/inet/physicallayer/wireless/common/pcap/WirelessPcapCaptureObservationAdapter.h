//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_WIRELESSPCAPCAPTUREOBSERVATIONADAPTER_H
#define __INET_WIRELESSPCAPCAPTUREOBSERVATIONADAPTER_H

#include "inet/common/packet/recorder/IPcapCaptureAdapter.h"

namespace inet {
namespace physicallayer {

class INET_API WirelessPcapCaptureObservationAdapter : public IPcapCaptureObservationAdapter
{
  public:
    virtual std::optional<PcapCaptureObservation> tryCreateObservation(const cObject *object, Direction direction) const override;
};

} // namespace physicallayer
} // namespace inet

#endif
