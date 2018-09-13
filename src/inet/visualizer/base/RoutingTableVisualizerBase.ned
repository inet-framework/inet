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
// This is a base module for routing table visualizer simple modules. It displays
// routes in the routing tables of the network. Routes present are not displayed
// by default, see the module parameters for how to enable the visualization.
// The actual visualization of routes is done in derived modules.
//
// This module subscribes for routeAdded, routeChanged, routeDeleted,
// interfaceIpv4ConfigChanged on the module determined by the visualizationSubjectModule
// parameter.
//
// The format string can contain the following directives:
//  - %m mask
//  - %g gateway
//  - %d destination
//  - %e metric
//  - %n interface name
//  - %i IRoute::info()
//  - %s IRoute::str()
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableOsgVisualizer, ~IRoutingTableVisualizer, ~VisualizerBase
//
simple RoutingTableVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayRoutingTables = default(false); // display routing table data, disabled by default
        bool displayRoutesIndividually = default(true); // display routes between the same network nodes individually, enabled by default
        bool displayLabels = default(true); // display labels on arrows, enabled by default

        string destinationFilter = default("*"); // which destinations are considered, matches all destinations by default
        string nodeFilter = default("*"); // which nodes are considered, matches all nodes by default

        string lineColor = default("black"); // route line color, black by default
        string lineStyle = default("solid"); // route line style (solid, dashed, dotted)
        double lineWidth = default(2); // route line width

        double lineShift = default(16); // line shift to avoid overlapping lines
        string lineShiftMode = default("normal"); // determines how overlapping lines are shifted, possible values are: none, normal, x, y, z; optional prefix + or -

        double lineContactSpacing = default(3); // spacing between arrow end and submodule icon
        string lineContactMode @enum("circular", "rectangular") = default("rectangular"); // determines how arrows are clipped at the submodules

        string labelFormat = default("%d/%m -> %g (%n)"); // determines what route data is displayed
        string labelFont = default("<default>, 8px"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        @class(RoutingTableVisualizerBase);
}

