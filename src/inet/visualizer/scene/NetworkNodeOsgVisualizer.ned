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

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// This module visualizes multiple network nodes on a 3D osg scene. The visual
// representation is either a 3D external model or a 2D icon that is rotated
// automatically towards the camera. The 3D external model is specified in the
// osgModel module parameter of the network node. The 2D icon is the image
// specified in the display string of the network node. The visualizer maintains
// a NetworkNodeOsgVisualization instance for each network node which can be
// annotated by other visualizers.
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeOsgVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        @class(NetworkNodeOsgVisualizer);
        bool displayModuleName = default(true); // display network node name, enabled by default
}

