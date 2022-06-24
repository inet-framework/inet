//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRACINGOBSTACLELOSS_H
#define __INET_ITRACINGOBSTACLELOSS_H

#include "inet/environment/contract/IPhysicalObject.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IObstacleLoss.h"

namespace inet {

namespace physicallayer {

class INET_API ITracingObstacleLoss : public IObstacleLoss
{
  public:
    class INET_API ObstaclePenetratedEvent : public cObject {
      public:
        const physicalenvironment::IPhysicalObject *object;
        const Coord intersection1;
        const Coord intersection2;
        const Coord normal1;
        const Coord normal2;
        const double loss;

      public:
        ObstaclePenetratedEvent(const physicalenvironment::IPhysicalObject *object, Coord intersection1, Coord intersection2, Coord normal1, Coord normal2, double loss);
    };

  public:
    static simsignal_t obstaclePenetratedSignal;
};

} // namespace physicallayer

} // namespace inet

#endif

