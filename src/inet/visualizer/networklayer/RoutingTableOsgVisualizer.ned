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

package inet.visualizer.networklayer;

import inet.visualizer.base.RoutingTableVisualizerBase;
import inet.visualizer.contract.IRoutingTableVisualizer;

//
// This module visualizes routes in the routing tables on a 3D osg scene. It
// displays an arrow for each route in the routing tables of the network. The
// arrow points from the node containing the routing table towards the gateway.
// The arrow automatically follows the movement of mobile nodes.
//
// @see ~RoutingTableCanvasVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
simple RoutingTableOsgVisualizer extends RoutingTableVisualizerBase like IRoutingTableVisualizer
{
    parameters:
        @class(RoutingTableOsgVisualizer);
}

