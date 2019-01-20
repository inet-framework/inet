//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSGEARTHGROUND_H
#define __INET_OSGEARTHGROUND_H

#include "inet/environment/contract/IGround.h"

#if defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)
//TODO the visualizers needed only for get the map from SceneOsgEarthVisualizer

#include <osgEarth/ElevationQuery>
#include <osgEarth/MapNode>
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

namespace inet {

namespace physicalenvironment {

class INET_API OsgEarthGround : public IGround, public cModule
{
  protected:
    osgEarth::Map *map = nullptr;
    osgEarth::ElevationQuery *elevationQuery = nullptr;
    IGeographicCoordinateSystem *coordinateSystem = nullptr;

    virtual void initialize() override;

  public:
    virtual Coord computeGroundProjection(const Coord &position) const override;
    virtual Coord computeGroundNormal(const Coord &position) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif // defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)

#endif // ifndef __INET_OSGEARTHGROUND_H

