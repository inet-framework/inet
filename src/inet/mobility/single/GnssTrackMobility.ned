//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Mobility model that follows a recorded GNSS (WGS84) track read from a CSV, GPX, or
// GeoJSON LineString source, mapping geographic positions into the scene through
// the `coordinateSystemModule`. Waypoints are interpolated along the great circle,
// so the path stays on the Earth's surface even between far-apart points, which
// works for both cars and aircraft.
//
// Timing is auto-detected: if the source carries per-point timestamps (a 4th CSV
// column, GPX `<time>` elements, or a GeoJSON `coordTimes` property) they drive the
// motion; otherwise the `speed` parameter traverses the track at a constant ground
// speed. `trackStartTime` sets when playback begins, and `loopMode` controls
// whether the track plays once, repeats from the start, or ping-pongs back and
// forth. With `faceForward` the body +X axis is aligned with the direction of
// travel (so e.g. a directional antenna mounted along it points along the track).
//
// The CSV or GPX track is given as raw data in the `trackData` string parameter (e.g.
// `trackData = readFile("track.csv")`). CSV rows are `latitude,longitude[,altitude[,time]]`
// (degrees, meters, seconds); blank lines, `#` comments, and a non-numeric header row are
// ignored. GeoJSON is supplied separately through the `trackJson` object parameter using
// `readJSON("track.geojson")`.
//
simple GnssTrackMobility extends MovingMobilityBase
{
    parameters:
        string trackData = default(""); // Raw CSV or GPX track data (use readFile("...") to load it from a file); used unless a GeoJSON track is given
        object trackJson = default(nullptr); // GeoJSON track, e.g. readJSON("track.geojson"); takes precedence over trackData
        string trackFormat @enum("auto","csv","gpx","geojson") = default("auto"); // Track source format; auto-detected from trackJson and the data content by default
        double trackStartTime @unit(s) = default(0s); // Simulation time at which playback of the first waypoint begins
        double speed @unit(mps) = default(nan mps); // Constant ground speed used only when the track has no timestamps
        string loopMode @enum("none","repeat","pingPong") = default("none"); // Playback looping: play once, repeat from start, or ping-pong
        displayStringTextFormat = default("{geo_position}\np: {position}\nv: {velocity}"); // Show the geographic latitude/longitude/altitude position string ({geo_position}) above the raw scene position and velocity
        @class(GnssTrackMobility);
}
