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

package inet.visualizer.power;

import inet.visualizer.contract.IEnergyStorageVisualizer;

//
// This module integrates canvas and osg energyStorage visualizers into a single module.
//
// @see ~EnergyStorageCanvasVisualizer, ~EnergyStorageOsgVisualizer, ~EnergyStorageVisualizerBase, ~IEnergyStorageVisualizer
//
module EnergyStorageVisualizer like IEnergyStorageVisualizer
{
    parameters:
        @display("i=block/app_s");
        string canvasVisualizerType = default("EnergyStorageCanvasVisualizer");
        string osgVisualizerType = default("EnergyStorageOsgVisualizer");
    submodules:
        canvasVisualizer: <canvasVisualizerType> like IEnergyStorageVisualizer if canvasVisualizerType != "" {
            parameters:
                @display("p=100,50");
        }
        osgVisualizer: <osgVisualizerType> like IEnergyStorageVisualizer if osgVisualizerType != "" {
            parameters:
                @display("p=100,150");
        }
}
