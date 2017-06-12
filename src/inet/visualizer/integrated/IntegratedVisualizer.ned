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

package inet.visualizer.integrated;

import inet.visualizer.contract.IIntegratedVisualizer;

//
// This module integrates canvas and osg visualizers into a single module.
//
module IntegratedVisualizer like IIntegratedVisualizer
{
    parameters:
        @display("i=block/app_s");
        string canvasVisualizerType = default("IntegratedCanvasVisualizer");
        string osgVisualizerType = default(""); // TODO:  revive "IntegratedOsgVisualizer" when OSG visualizers are at least as good as Canvas based
    submodules:
        canvasVisualizer: <canvasVisualizerType> like IIntegratedVisualizer if canvasVisualizerType != "" {
            parameters:
                @display("p=100,50");
        }
        osgVisualizer: <osgVisualizerType> like IIntegratedVisualizer if osgVisualizerType != "" {
            parameters:
                @display("p=100,150");
        }
}
