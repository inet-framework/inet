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
// It keeps track of wireless interface associations by subscribing to a number
// of signals at the provided subscription module.
//
// This module subscribes for NF_L2_ASSOCIATED, NF_L2_DISASSOCIATED,
// NF_L2_AP_ASSOCIATED,NF_L2_AP_DISASSOCIATED on the module determined by the
// subscriptionModule parameter.
//
// @see ~Ieee80211CanvasVisualizer, ~Ieee80211OsgVisualizer, ~IIeee80211Visualizer, ~VisualizerBase
//
simple Ieee80211VisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // module path to the network node visualizer module, refers to a sibling submodule by default
        string subscriptionModule = default("<root>"); // module where the visualizer subscribes for interface changed signals, root module by default
        
        bool displayAssociations = default(false); // display association icon and SSID, disabled by default

        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default

        string icon = default("misc/signal_s"); // wireless network visualization icon
        string iconColor @enum("light", "dark") = default("dark"); // wireless network icon color is a list of colors, a set of dark colors by default
        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(Ieee80211VisualizerBase);
}
