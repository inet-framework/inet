//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.SceneVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

//
// Visualizes a street map from an OpenStreetMap (openstreetmap.org) file on a 2D canvas.
// Provides a realistic geographic background for network simulations.
//
// The visualization includes:
// - Roads, buildings, and other map features from the OpenStreetMap file TODO?
// - Geographic positioning of network nodes based on the coordinate system
// - Automatic adjustment of the background box to match map bounds
//
// Particularly useful for simulations that model real-world geographic scenarios,
// such as vehicular networks, urban wireless deployments, or any simulation where
// geographic context is important.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
// @see ~IGeographicCoordinateSystem, ~SimpleGeographicCoordinateSystem, ~OsgGeographicCoordinateSystem
//
simple OpenStreetMapSceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(OpenStreetMapSceneCanvasVisualizer);
        string coordinateSystemModule = default("coordinateSystem"); // The geographic coordinate system, refers to a top-level module by default

        xml mapFile; // OpenStreetMap map file, e.g. one exported from openstreetmap.org

        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        bool adjustBackgroundBox = default(true); // If true, sets the background box (bgb tag) to match map bounds
}
