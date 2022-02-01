//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSGEARTHGROUND_H
#define __INET_OSGEARTHGROUND_H

#include "inet/environment/contract/IGround.h"

#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)
// TODO the visualizers needed only for get the map from SceneOsgEarthVisualizer

#include <osgEarth/ElevationQuery>
#include <osgEarth/MapNode>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

namespace inet {

namespace physicalenvironment {

class INET_API OsgEarthGround : public IGround, public cModule
{
  protected:
    osgEarth::Map *map = nullptr;
    osgEarth::ElevationQuery *elevationQuery = nullptr;
    ModuleRefByPar<IGeographicCoordinateSystem> coordinateSystem;

    virtual void initialize() override;

  public:
    virtual Coord computeGroundProjection(const Coord& position) const override;
    virtual Coord computeGroundNormal(const Coord& position) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif // defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)

#endif

