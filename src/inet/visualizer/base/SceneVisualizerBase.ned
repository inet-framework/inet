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

package inet.visualizer.base;

//
// This is a base module for scene visualizer simple modules. It determines the
// bounds of the scene, that is the minimum area where the whole simulation
// fits in. The scene encloses all physical objects and the constraint area
// of all mobile nodes. The visualizer also provides a timer which can be used
// to periodically update the user interface independently of the underlying
// simulation. The actual visualization of the scene is done in derived modules.
//
// @see ~SceneCanvasVisualizer, ~SceneOsgVisualizer, ~ISceneVisualizer, ~VisualizerBase
//
simple SceneVisualizerBase extends VisualizerBase
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // the physical environment module, refers to a top level submodule by default
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer, refers to a sibling submodule by default
        double axisLength @unit(m) = default(nan m); // length of x, y, and z axes, no axis by default
        double animationSpeed = default(0); // always active animation speed independent of any visualization, value must be in the range (0, +inf)
        double sceneMinX @unit(m) = default(nan m); // lower bound of x coordinates in scene
        double sceneMinY @unit(m) = default(nan m); // lower bound of y coordinates in scene
        double sceneMinZ @unit(m) = default(nan m); // lower bound of z coordinates in scene
        double sceneMaxX @unit(m) = default(nan m); // upper bound of x coordinates in scene
        double sceneMaxY @unit(m) = default(nan m); // upper bound of y coordinates in scene
        double sceneMaxZ @unit(m) = default(nan m); // upper bound of z coordinates in scene
        @class(SceneVisualizerBase);
}
