//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FACINGMOBILITY_H
#define __INET_FACINGMOBILITY_H

#include "inet/common/ModuleRef.h"
#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API FacingMobility : public MobilityBase
{
  protected:
    ModuleRef<IMobility> sourceMobility;
    ModuleRef<IMobility> targetMobility;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }
    virtual void handleParameterChange(const char *name) override;

  public:
    virtual const Coord& getCurrentPosition() override { return lastPosition; }
    virtual const Coord& getCurrentVelocity() override { return Coord::ZERO; }
    virtual const Coord& getCurrentAcceleration() override { return Coord::ZERO; }

    virtual const Quaternion& getCurrentAngularPosition() override;
    virtual const Quaternion& getCurrentAngularVelocity() override { return Quaternion::NIL; }
    virtual const Quaternion& getCurrentAngularAcceleration() override { return Quaternion::NIL; }
};

} // namespace inet

#endif

