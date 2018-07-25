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

package inet.visualizer.common;

import inet.visualizer.base.StatisticVisualizerBase;
import inet.visualizer.contract.IStatisticVisualizer;

//
// This module visualizes a statistic on a 3D osg scene. See the base module for
// how to configure which statistic is displayed. If configured properly, then
// it displays the last value of the statistic just above the visualization of
// the corresponding network node. The value and the unit is wrapped in a filled
// rectangle.
//
// @see ~StatisticCanvasVisualizer, ~StatisticVisualizer, ~StatisticVisualizerBase, ~IStatisticVisualizer
//
simple StatisticOsgVisualizer extends StatisticVisualizerBase like IStatisticVisualizer
{
    parameters:
        @class(StatisticOsgVisualizer);
}

