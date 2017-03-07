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

import inet.visualizer.contract.INetworkConnectionVisualizer;

//
// This module integrates canvas and osg connection visualizers into a single module.
//
// @see ~NetworkConnectionCanvasVisualizer, ~NetworkConnectionOsgVisualizer, ~NetworkConnectionVisualizerBase, ~INetworkConnectionVisualizer
//
module NetworkConnectionVisualizer like INetworkConnectionVisualizer
{
    parameters:
        @display("i=block/app_s");
        string canvasVisualizerType = default("NetworkConnectionCanvasVisualizer");
        string osgVisualizerType = default("NetworkConnectionOsgVisualizer");
    submodules:
        canvasVisualizer: <> like INetworkConnectionVisualizer if canvasVisualizerType != "" {
            parameters:
                @display("p=100,50");
        }
        osgVisualizer: <> like INetworkConnectionVisualizer if osgVisualizerType != "" {
            parameters:
                @display("p=100,150");
        }
}
