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
// This is a base module for link break visualizer simple modules. It keeps
// track of link breaks in a network. Link breaks are not displayed by
// default, see module parameters for how to enable the visualization. The
// actual visualization of link breaks is done in derived modules.
//
// This module subscribes for NF_LINK_BREAK on the module determined by the
// subscriptionModule parameter. If the visualizer receives a link break signal,
// then it stores the broken link and starts the link break visualization.
//
// @see ~LinkBreakCanvasVisualizer, ~LinkBreakOsgVisualizer, ~ILinkBreakVisualizer, ~LinkBreakVisualizerBase
//
simple LinkBreakVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for link break signals, root module by default
        string nodeFilter = default(""); // which network nodes are considered to display link breaks, matches no nodes by default
        string icon = default("status/cross");
        double iconTintAmount = default(0);
        string iconTintColor = default("");
        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime");
        double fadeOutHalfLife @unit(s) = default(1s); // how quickly link breaks fade away, opacity halves every second by default
        @class(LinkBreakVisualizerBase);
}
