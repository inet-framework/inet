//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.geographic;

import inet.mobility.static.StationaryMobility;

//
// Mobility for stationary nodes placed by geographic coordinates (latitude,
// longitude, altitude) in a geocentric (WGS84/ECEF) scene, such as a
// ~GeocentricCoordinateSystem. The node's scene position is fixed at its
// geographic location, which is an Earth-radius-scale ECEF coordinate and is
// therefore not meaningful to plot on the flat 2D scene.
//
// Unlike plain ~StationaryMobility, this model decouples the on-canvas module
// icon from that scene coordinate: the icon stays at its display-string position
// (set it with `@display("p=...")`), and the scene position is taken solely from
// `initialLatitude`/`initialLongitude`/`initialAltitude`. The display string is
// neither initialized from, written to, nor read back into the scene position, so
// the icon does not jump to extreme coordinates and the geographic position is not
// corrupted by the icon position. A map visualizer is the proper view of the
// actual location.
//
// It is a parameter specialization of ~StationaryMobility (same C++ behavior).
//
simple GeographicStationaryMobility extends StationaryMobility
{
    parameters:
        initFromDisplayString = default(false);   // take the position from latitude/longitude, not the display string
        updateDisplayString = default(false);     // keep the icon at its display-string position; do not move it to the ECEF coordinate
        updateFromDisplayString = default(false);  // do not read the icon position back into the scene position
        displayStringTextFormat = default("%g\np: %p"); // Show the geographic (latitude/longitude/altitude) position above the scene position
        @class(StationaryMobility);
}
