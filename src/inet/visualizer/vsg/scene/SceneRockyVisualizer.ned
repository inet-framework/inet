//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.vsg.scene;

//
// Renders real geospatial imagery and terrain in the VSG 3D view using Rocky
// (the VSG successor to osgEarth). Extends ~SceneVsgVisualizer, so all the usual
// scene setup (axes, viewpoint, optional floor) still applies; on top of it the
// Rocky map is built against the live VSG viewer and placed at the scene origin,
// anchored to the geographic origin of a geographic coordinate system. Requires
// INET to be built with WITH_ROCKY=yes.
//
module SceneRockyVisualizer extends SceneVsgVisualizer
{
    parameters:
        @class(SceneRockyVisualizer);
        string coordinateSystemModule; // path of a module implementing IGeographicCoordinateSystem (scene <-> lat/lon/alt)
        string imageFile = default(""); // a GDAL-openable imagery source (GeoTIFF/...) for the map's color layer; empty = none
        string elevationFile = default(""); // a GDAL-openable elevation source (GeoTIFF DEM/...) for the terrain; empty = flat
        string imageUrl = default(""); // a TMS imagery endpoint for online map tiles, e.g. readymap.org; empty = none (needs network at runtime)
        string elevationUrl = default(""); // a TMS elevation endpoint for online terrain tiles; empty = none (needs network at runtime)
        string rockyResourcePath = default(""); // sets ROCKY_FILE_PATH so Rocky finds its shaders (point at the Rocky install's share/rocky); empty = leave the environment as-is
}
