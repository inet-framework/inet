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

import inet.visualizer.base.SceneVisualizerBase;
import inet.visualizer.contract.ISceneVisualizer;

//
// This module visualizes a stream map given in an OpenStreetMap file.
// Other objects of the scene are placed on the map according to the coordinate system module.
//
// @see ~SceneCanvasVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
// @see ~IGeographicCoordinateSystem, ~SimpleGeographicCoordinateSystem, ~OsgGeographicCoordinateSystem 
//
simple OpenStreetMapSceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        @class(OpenStreetMapSceneCanvasVisualizer);
        string coordinateSystemModule = default("coordinateSystem"); // the geographic coordinate system, refers to a top level module by default
 
        xml mapFile; // OpenStreetMap map file, e.g. one exported from openstreetmap.org

        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        bool adjustBackgroundBox = default(true); // if true, sets background box (bgb tag) to match map bounds 
}

