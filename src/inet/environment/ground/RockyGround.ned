//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.common.Module;
import inet.environment.contract.IGround;

//
// Models a ground surface from real geospatial elevation data using Rocky (the
// VSG-based successor to osgEarth). Elevation is read from a GDAL-openable source
// (e.g. a local GeoTIFF DEM) and sampled with rocky::ElevationSampler; scene
// positions are mapped to geographic coordinates through a geographic coordinate
// system, sampled, and mapped back. Headless (no rendering) and independent of
// OSG. Requires INET to be built with WITH_ROCKY=yes.
//
// The ~TwoRayGroundReflection path loss and any other ~IGround consumer then work
// over the real terrain with no changes to them, exactly as with ~PointCloudGround.
//
module RockyGround extends Module like IGround
{
    parameters:
        @class(RockyGround);
        string coordinateSystemModule; // path of a module implementing IGeographicCoordinateSystem (scene <-> lat/lon/alt)
        string elevationFile; // a GDAL-openable elevation source (GeoTIFF/DEM/...), local file or URL
        double outOfBoundsElevation @unit(m) = default(0m); // elevation returned where the source has no data
}
