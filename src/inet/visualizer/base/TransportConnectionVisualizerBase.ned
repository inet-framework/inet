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
// This is a base module for transport connection visualizer simple modules. It
// keeps track of open transport connections in a network. Open connections are
// not displayed by default, see the module parameters for how to enable the
// visualization. The actual visualization of transport connections is done
// in derived modules.
//
// This module subscribes for tcpConnectionAdded and tcpConnectionRemoved on the
// module determined by the visualizationSubjectModule parameter.
//
// @see ~TransportConnectionCanvasVisualizer, ~TransportConnectionOsgVisualizer, ~ITransportConnectionVisualizer, ~VisualizerBase
//
simple TransportConnectionVisualizerBase extends VisualizerBase
{
    parameters:
        string networkNodeVisualizerModule = default("^.networkNodeVisualizer"); // the network node visualizer module, refers to a sibling submodule by default

        bool displayTransportConnections = default(false);// display transport connection end points, disabled by default

        string sourceNodeFilter = default("*"); // which source network nodes are considered to display connections, matches no nodes by default
        string sourcePortFilter = default("*"); // which source ports are considered to display connections, matches all ports by default
        string destinationNodeFilter = default("*"); // which destination network nodes are considered to display connections, matches no nodes by default
        string destinationPortFilter = default("*"); // which destination ports are considered to display connections, matches all ports by default

        string icon = default("misc/marker_s"); // transport connection visualization icon
        string iconColor @enum("light", "dark") = default("dark"); // transport connection icon color is a list of colors, a set of dark colors by default
        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default
        string placementHint = default("top"); // annotation placement hint, space separated list of any, top, bottom, left, right, topLeft, topCenter, topRight, etc.
        double placementPriority = default(0); // determines the order of annotation positioning

        @class(TransportConnectionVisualizerBase);
}

