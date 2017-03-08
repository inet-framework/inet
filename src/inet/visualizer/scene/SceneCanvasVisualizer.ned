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
// This module visualizes the scene on a 2D canvas. It provides the 3D coordinate
// system's 2D projection for other visualizers. This is used to get a consistent
// visualization accross all visualizers. It also displays the coordinate system
// axes.
//
// @see ~SceneOsgVisualizer, ~SceneVisualizer, ~SceneVisualizerBase, ~ISceneVisualizer
//
simple SceneCanvasVisualizer extends SceneVisualizerBase like ISceneVisualizer
{
    parameters:
        double zIndex = default(0); // determines the drawing order of figures relative to other visualizers
        string viewAngle = default("z"); // view angle is either x, y, z, isometric, isometricN (where 0 <= N < 48) or three rotation angles separated by spaces (it uses orthographic projection that is the projection is parallel to the normal of the viewing surface)
        string viewScale = default("1 1"); // the 2D scale is applied after the view projection
        string viewTranslation = default("0 0"); // the 2D translation is applied after the view projection
        string descriptionFigure = default(""); // if not empty, it should be the name of a text figure; it will display the active configuration's description 
        @class(SceneCanvasVisualizer);
}
