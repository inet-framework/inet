//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ATTACHEDMOBILITY_H
#define __INET_ATTACHEDMOBILITY_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API AttachedMobility : public MobilityBase, public cListener
{
  protected:
    ModuleRefByPar<IMobility> mobility;
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

