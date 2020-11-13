//
// Copyright (C) 2006 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

