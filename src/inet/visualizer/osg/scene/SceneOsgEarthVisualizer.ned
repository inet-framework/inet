//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.osg.base.SceneOsgVisualizerBase;

//
// Visualizes the simulation scene on a realistic 3D Earth model using osgEarth.
// Integrates geographic data with network simulation for location-based scenarios.
//
// The visualization includes:
// - Realistic 3D terrain, satellite imagery, and map features
// - Proper geographic positioning of network elements
// - Integration with coordinate systems for accurate placement
// - Support for various map formats through external map files
//
// The network simulation is placed on the Earth model according to the specified
// geographic coordinate system, allowing for realistic visualization of networks
// in their actual geographic context.
//
// Particularly useful for simulating networks with geographic dependencies,
// visualizing coverage areas in real-world locations, analyzing terrain effects
// on wireless propagation, and creating compelling demonstrations of location-based
// network applications.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
// @see ~IGeographicCoordinateSystem, ~SimpleGeographicCoordinateSystem, ~OsgGeographicCoordinateSystem
//
simple SceneOsgEarthVisualizer extends SceneOsgVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(SceneOsgEarthVisualizer);
        string coordinateSystemModule = default("coordinateSystem"); // The geographic coordinate system, refers to a top level module by default
        string mapFile; // Map file, e.g. some part of the earth, mandatory parameter
}
