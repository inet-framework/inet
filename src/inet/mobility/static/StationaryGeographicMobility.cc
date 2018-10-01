//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/mobility/static/StationaryGeographicMobility.h"

namespace inet {

Define_Module(StationaryGeographicMobility);

void StationaryGeographicMobility::initializePosition()
{
    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
    auto initialLatitude = deg(par("initialLatitude"));
    auto initialLongitude = deg(par("initialLongitude"));
    auto initialAltitude = m(par("initialAltitude"));
    GeoCoord geoPositon(initialLatitude, initialLongitude, initialAltitude);
    lastPosition = coordinateSystem->computeSceneCoordinate(geoPositon);
}

void StationaryGeographicMobility::initializeOrientation()
{
    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
    auto sceneOrientation = coordinateSystem->getSceneOrientation();
    auto initialHeading = deg(par("initialHeading"));
    auto initialElevation = deg(par("initialElevation"));
    auto initialBank = deg(par("initialBank"));
    Quaternion initialOrientation(EulerAngles(deg(90) - initialHeading, -initialElevation, initialBank));
    // TODO: this is probably wrong, but fixing it is delayed until we have an example
    lastOrientation = initialOrientation / sceneOrientation;
}

} // namespace inet

