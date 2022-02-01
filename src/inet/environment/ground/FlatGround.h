//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FLATGROUND_H
#define __INET_FLATGROUND_H

#include "inet/environment/contract/IGround.h"

namespace inet {

namespace physicalenvironment {

class INET_API FlatGround : public IGround, public cModule
{
  protected:
    double elevation = NaN;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual Coord computeGroundProjection(const Coord& position) const override;
    virtual Coord computeGroundNormal(const Coord& position) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif

