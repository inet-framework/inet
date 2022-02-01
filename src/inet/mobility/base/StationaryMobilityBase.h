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
    virtual void handleSelfMessage(cMessage *message) override { throw cRuntimeError("Invalid operation"); }

  public:
    virtual const Quaternion& getCurrentAngularPosition() override { return lastOrientation; }
    virtual const Quaternion& getCurrentAngularVelocity() override { return Quaternion::IDENTITY; }
    virtual const Quaternion& getCurrentAngularAcceleration() override { return Quaternion::IDENTITY; }

    virtual const Coord& getCurrentPosition() override { return lastPosition; }
    virtual const Coord& getCurrentVelocity() override { return Coord::ZERO; }
    virtual const Coord& getCurrentAcceleration() override { return Coord::ZERO; }

    virtual double getMaxSpeed() const override { return 0; }

    virtual const Coord& getConstraintAreaMax() const override { return lastPosition; }
    virtual const Coord& getConstraintAreaMin() const override { return lastPosition; }
};

} // namespace inet

#endif

