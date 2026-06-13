//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.geographic;

import inet.mobility.base.MovingMobilityBase;

//
// Mobility model that propagates a satellite orbit from a TLE (two-line element)
// set using the embedded SGP4 implementation, and reports the position in the
// simulation scene through the geographic coordinate system referenced by
// `coordinateSystemModule` (typically a ~GeocentricCoordinateSystem).
//
// The satellite is selected from `tleFile` by `satelliteIndex`, `satelliteName`,
// or `satelliteCatalogNumber` (whichever is given, in that order; otherwise the
// first satellite). `epoch` is the absolute UTC time mapped to simulation time 0;
// if empty, the satellite's own TLE epoch is used. A ~SatelliteController normally
// sets `epoch` (and the TLE selection) on the satellites it creates.
//
// See ~GeocentricCoordinateSystem for the scene frame and Wgs84 for the
// (GMST-only) Earth-orientation conventions.
//
simple SatelliteMobility extends MovingMobilityBase
{
    parameters:
        string tleFile @mutable = default(""); // Path to the TLE (two-line element) file (set directly, or injected by a SatelliteController)
        int satelliteIndex @mutable = default(-1); // Index of the satellite within the TLE file (-1: use name/catalog number)
        string satelliteName @mutable = default(""); // Name of the satellite (the line preceding the element set)
        string satelliteCatalogNumber @mutable = default(""); // NORAD catalog number of the satellite
        string epoch @mutable = default(""); // Absolute UTC time (ISO-8601, e.g. "2026-06-02T00:00:00Z") mapped to simulation time 0; empty uses the TLE epoch
        string pointing @enum("none", "nadir", "velocity") = default("nadir"); // Satellite attitude: none, body +Z toward nadir, or body +X along velocity
        updateInterval = default(1s); // Satellites move continuously; update the state once per second by default
        faceForward = default(false); // Orientation is controlled by the 'pointing' parameter
        updateDisplayString = default(false); // A satellite's scene position is geocentric (ECEF) and not meaningful to plot on the flat 2D scene, so leave the node icon at its layout position (the map visualizer shows the real position); set true to track it on the scene
        @class(SatelliteMobility);
}

