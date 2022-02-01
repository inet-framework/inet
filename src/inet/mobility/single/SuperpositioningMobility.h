//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SUPERPOSITIONINGMOBILITY_H
#define __INET_SUPERPOSITIONINGMOBILITY_H

#include "inet/mobility/base/MobilityBase.h"

namespace inet {

class INET_API SuperpositioningMobility : public MobilityBase, public cListener
{
  protected:
    enum class PositionComposition {
        PC_UNDEFINED = -1,
        PC_ZERO,
        PC_SUM,
        PC_AVERAGE,
        PC_ELEMENT,
    };

    enum class OrientationComposition {
        OC_UNDEFINED = -1,
        OC_ZERO,
        OC_SUM,
        OC_AVERAGE,
        OC_FACE_FORWARD,
        OC_ELEMENT,
    };

    Coord lastVelocity;
    Coord lastAcceleration;
    Quaternion lastAngularVelocity;
    Quaternion lastAngularAcceleration;

  protected:
    PositionComposition positionComposition = PositionComposition::PC_UNDEFINED;
    OrientationComposition orientationComposition = OrientationComposition::OC_UNDEFINED;
    int positionElementIndex = -1;
    int orientationElementIndex = -1;
    std::vector<IMobility *> elements;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }
    virtual void setInitialPosition() override;

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

