//
// Copyright (C) 2020 OpenSim Ltd.
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

#ifndef __INET_ATTACHEDMOBILITY_H
#define __INET_ATTACHEDMOBILITY_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API AttachedMobility : public MobilityBase, public cListener
{
  protected:
    IMobility *mobility = nullptr;
    Coord positionOffset = Coord::NIL;
    Quaternion orientationOffset = Quaternion::NIL;
    bool isZeroOffset = false;
    Coord lastVelocity;
    Quaternion lastAngularPosition;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }

  public:
    virtual const Coord& getCurrentPosition() override;
    virtual const Coord& getCurrentVelocity() override;
    virtual const Coord& getCurrentAcceleration() override;

    virtual const Quaternion& getCurrentAngularPosition() override;
    virtual const Quaternion& getCurrentAngularVelocity() override;
    virtual const Quaternion& getCurrentAngularAcceleration() override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

