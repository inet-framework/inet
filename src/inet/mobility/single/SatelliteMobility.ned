//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Mobility model that propagates a satellite orbit from a TLE (two-line element)
// set using the embedded SGP4 implementation, and reports the position in the
// simulation scene through the geographic coordinate system referenced by
// `coordinateSystemModule` (typically a ~Wgs84EcefGeographicCoordinateSystem); when
// `coordinateSystemModule` is empty, the position is reported in raw geocentric ECEF coordinates.
//
// The TLE data is taken from the `tleData` string parameter (the raw two-line element
// text, e.g. `tleData = readFile("satellites.tle")`), or from a file named by `tleFile`
// (when set, `tleData` defaults to the file's contents). The satellite is selected from it
// by `satelliteIndex`, `satelliteName`, or `satelliteCatalogNumber` (whichever is given,
// in that order; otherwise the first satellite). `epoch` is the absolute UTC time mapped
// to simulation time 0; if empty, the satellite's own TLE epoch is used. Each satellite
// is configured individually by setting these parameters directly.
//
// See ~Wgs84EcefGeographicCoordinateSystem for the scene frame and Wgs84 for the
// (GMST-only) Earth-orientation conventions.
//
simple SatelliteMobility extends MovingMobilityBase
{
    parameters:
        string tleFile = default(""); // Path to a TLE file; when non-empty, tleData defaults to its contents. A relative path resolves against this NED file's directory, so pass an absolute path (e.g. tleFile = absFilePath("sats.tle") in the ini, which resolves relative to that ini)
        string tleData = default(tleFile != "" ? readFile(tleFile) : ""); // Raw TLE (two-line element) data; use readFile("...") to load it inline, or set tleFile instead
        int satelliteIndex = default(-1); // Index of the satellite within the TLE file (-1: use name/catalog number)
        string satelliteName @mutable = default(""); // Name of the satellite (the line preceding the element set); also used as the label shown by the geographic map/sky-view visualizers. @mutable because the module writes the resolved name back here when the satellite is selected by index or catalog number
        string satelliteCatalogNumber = default(""); // NORAD catalog number of the satellite
        string epoch = default(""); // Absolute UTC time (ISO-8601, e.g. "2026-06-02T00:00:00Z") mapped to simulation time 0; empty uses the TLE epoch
        string attitudeMode @enum("earth_fixed", "star_fixed", "nadir", "zenith", "velocity") = default("nadir"); // Aim of the body +X axis: earth_fixed (held fixed in the rotating ECEF scene frame), star_fixed (fixed in the inertial ECI frame, i.e. star-pointing), nadir (toward Earth center), zenith (radially outward), or velocity (along the velocity vector). This fully determines the body orientation, so the inherited `faceForward` parameter has no effect
        updateInterval = default(1s); // Satellites move continuously; update the state once per second by default
        updateDisplayString = default(false); // A satellite's scene position is geocentric (ECEF) and not meaningful to plot on the flat 2D scene, so leave the node icon at its layout position (the map visualizer shows the real position); set true to track it on the scene
        displayStringTextFormat = default("{geo_position}\np: {position}\nv: {velocity}"); // Show the geographic latitude/longitude/altitude position string ({geo_position}) above the raw ECEF scene position and velocity
        @class(SatelliteMobility);
}

