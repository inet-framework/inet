//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.scene;

import inet.visualizer.contract.ISceneVisualizer;
import inet.visualizer.osg.base.SceneOsgVisualizerBase;

//
// This module visualizes the scene using osgEarth. It displays a 3D osg map of
// some part of the earth as an integrated part of the 3D visualization of the
// simulation. The map is loaded from an external resource specified by the
// mapFile parameter. The scene is placed on the map according to the
// coordinate system module.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
// @see ~IGeographicCoordinateSystem, ~SimpleGeographicCoordinateSystem, ~OsgGeographicCoordinateSystem
//
simple SceneOsgEarthVisualizer extends SceneOsgVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(SceneOsgEarthVisualizer);
        string coordinateSystemModule = default("coordinateSystem"); // the geographic coordinate system, refers to a top level module by default
        string mapFile; // map file, e.g. some part of the earth, mandatory parameter
}

