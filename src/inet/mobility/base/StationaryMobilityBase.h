//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATIONARYMOBILITYBASE_H
#define __INET_STATIONARYMOBILITYBASE_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API StationaryMobilityBase : public MobilityBase
{
  protected:
    void handleSelfMessage(cMessage *message) override { throw cRuntimeError("Invalid operation"); }

  public:
    const Quaternion& getCurrentAngularPosition() override { return lastOrientation; }
    const Quaternion& getCurrentAngularVelocity() override { return Quaternion::IDENTITY; }
    const Quaternion& getCurrentAngularAcceleration() override { return Quaternion::IDENTITY; }

    const Coord& getCurrentPosition() override { return lastPosition; }
    const Coord& getCurrentVelocity() override { return Coord::ZERO; }
    const Coord& getCurrentAcceleration() override { return Coord::ZERO; }

    double getMaxSpeed() const override { return 0; }

    const Coord& getConstraintAreaMax() const override { return lastPosition; }
    const Coord& getConstraintAreaMin() const override { return lastPosition; }
};

} // namespace inet

#endif

