//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common.geometry.common;

//
// This module interface is used by geographic coordinate systems. A geographic
// coordinate system maps scene coordinates to geographic coordinates, and
// vice versa.
//
moduleinterface IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2");
}

//
// This module provides a very simple and less accurate geographic coordinate
// system without using OSG. It doesn't support orientation.
//
simple SimpleGeographicCoordinateSystem like IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2");
        double sceneLatitude @unit(deg); // Latitude coordinate of the scene origin
        double sceneLongitude @unit(deg); // Longitude coordinate of the scene origin
        double sceneAltitude @unit(m) = default(0m); // Altitude coordinate of the scene origin
}

//
// This module provides an accurate geographic coordinate system using the
// built-in OSG API.
//
simple OsgGeographicCoordinateSystem like IGeographicCoordinateSystem
{
    parameters:
        @display("i=block/table2");
        double sceneLatitude @unit(deg); // Latitude coordinate of the scene origin
        double sceneLongitude @unit(deg); // Longitude coordinate of the scene origin
        double sceneAltitude @unit(m) = default(0m); // Altitude coordinate of the scene origin
        double sceneHeading @unit(deg) = default(90deg); // Geographic heading of the scene (0deg means X axis points north, 90deg means X axis points east)
        double sceneElevation @unit(deg) = default(0deg); // Geographic elevation of the scene (0deg means X axis is horizontal, positive rotates X axis towards the sky)
        double sceneBank @unit(deg) = default(0deg); // Bank of the scene
}

