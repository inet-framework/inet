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
// This is a base module for statistic visualizer simple modules. It keeps track
// of the last value of a statistic for multiple network nodes. The actual
// visualization of statistic values is done in derived modules. It subscribes
// for the signal determined by the signalName parameter at the module specified
// by the subscriptionModule parameter. If the visualizer receives such a signal
// it checks if the source matches the sourcePathFilter parameter. If the signal
// passes the check it determines the last value of the statistic specified by
// the statisticName parameter.
//
// @see ~StatisticCanvasVisualizer, ~StatisticOsgVisualizer, ~IStatisticVisualizer, ~StatisticVisualizerBase
//
simple StatisticVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for the signal, root module by default
        string sourcePathFilter = default(""); // pattern that must match the full path of the source, no source filter by default
        string signalName = default(""); // signal name to subscribe, no signal name by default
        string statisticName = default(signalName); // statistic name to get the value, signal name by default
        string prefix = default(statisticName); // display this string as the prefix of the value, statistic name by default
        string unit = default(""); // display the value in this unit, statistic unit by default
        string fontColor = default("black"); // statistic value font color, black by default
        string backgroundColor = default("white"); // statistic value background color, white by default
        double opacity = default(0.5); // opacity of the visualization
        double minValue = default(-1/0); // minimum value of the statistic, negative infinity by default
        double maxValue = default(1/0); // maximum value of the statistic, positive infinity by default
        @class(StatisticVisualizerBase);
}
