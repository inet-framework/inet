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
// This is a base module for IEEE 802.11 association visualizer simple modules.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~IIeee80211Visualizer, ~Ieee80211VisualizerBase
//
simple Ieee80211VisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer");
        string subscriptionModule = default(""); // module path where the visualizer subscribes for interface changed signals, root module by default
        string nodeFilter = default(""); // which nodes are considered, matches no nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default
        string icon = default("misc/signal_s"); // wireless network visualization icon
        @class(Ieee80211VisualizerBase);
}
