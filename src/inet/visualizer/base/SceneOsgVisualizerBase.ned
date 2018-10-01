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
// This is a base module for scene visualizer simple modules on a 3D osg scene.
// It displays the scene either as a filled rectangle or as an image.
//
// @see ~SceneOsgVisualizer, ~SceneOsgEarthVisualizer, ~SceneVisualizerBase
//
simple SceneOsgVisualizerBase extends SceneVisualizerBase
{
    parameters:
        string cameraManipulator @enum("auto","trackball","overview","terrain","earth") = default("auto"); // type of camera manipulator
        double cameraDistanceFactor = default(5); // camera distance relative to the network bounding sphere radius
        double zNear @unit(m) = default(1m); // near distance for Z buffer optimization
        double zFar @unit(m) = default(100000m); // far distance for Z buffer optimization
        double fieldOfView @unit(deg) = default(30deg); // camera field of view
        string clearColor = default(""); // scene background color, not set by default

        bool displayScene = default(true); // display the scene, enabled by default
        string sceneColor = default("#A0E0A0"); // scene solid color if scene texture is not provided, gray by default
        double sceneOpacity = default(1.0); // scene opacity, opaque by default (not recommended to change)
        string sceneImage = default(""); // scene texture (repeated), no image by default
        double sceneImageSize @unit(m) = default(1m); // scene texture size
        bool sceneShading = default(true); // exponential shading for scene color, enabled by default

        @class(SceneOsgVisualizerBase);
}

