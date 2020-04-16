//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.visualizer.scene;

import inet.visualizer.base.SceneOsgVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

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

