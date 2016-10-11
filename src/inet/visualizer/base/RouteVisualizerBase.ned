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
// This is a base module for route visualizer simple modules. It keeps track of
// active routes in a network. A route between two network node is considered
// active if a packet is sent recently between their network layers. Active
// routes are not displayed by default, see the module parameters for how to
// enable the visualization. Note that routing tables may contain many inactive
// routes that are not in use, and thus they are not displayed. The actual
// visualization of active routes is done in derived modules.
//
// This module subscribes for packetReceivedFromUpperSignal,
// packetSentToUpperSignal, and packetReceivedFromLowerSignal on the module
// determined by the subscriptionModule parameter. If the visualizer receives a
// packetReceivedFromUpperSignal from a network protocol module, then it stores
// the corresponding packet id. If later on the visualizer receives a
// packetSentToUpperSignal from another network protocol module with the same
// packet id, then it adds an active route between the source and the
// destination nodes. During the route discovery if the visualizer receives a
// packetReceivedFromLowerSignal from a network protocol module, then it adds
// the node to the intermediate nodes of the route. After a certain amount of
// time the route becomes inactive unless it is reinforced by another packet. 
// This module also subscribes for mobilityStateChangedSignal to be able to
// follow the movement of mobile nodes.
//
// @see ~RouteCanvasVisualizer, ~RouteOsgVisualizer, ~IRouteVisualizer, ~RouteVisualizerBase
//
simple RouteVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for network layer signals, root module by default
        string packetNameFilter = default(""); // which packets are considered to determine active routes, matches no packets by default
        string lineColor = default(""); // route line color, automatic by default
        double lineWidth = default(3); // route line width
        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime");
        double fadeOutHalfLife @unit(s) = default(1s); // how quickly inactive routes fade away, opacity halves every second by default
        @class(RouteVisualizerBase);
}
