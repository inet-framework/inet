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
// This is a base module for packet drop visualizer simple modules. It keeps
// track of dropped packets in a network. Packet drops are not displayed by
// default, see module parameters for how to enable the visualization. The
// actual visualization of packet drops is done in derived modules.
//
// This module subscribes for packetFromLowerDropped and packetFromUpperDropped
// on the module determined by the subscriptionModule parameter. If the
// visualizer receives a packet dropped signal, then it stores the dropped
// packet and starts the drop visualization.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~IPacketDropVisualizer, ~VisualizerBase
//
simple PacketDropVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default("<root>"); // module where the visualizer subscribes for packet drop signals, root module by default

        bool displayPacketDrops = default(false); // display icons for packet drops, disabled by default

        string nodeFilter = default("*"); // which network nodes are considered, matches all nodes by default
        string interfaceFilter = default("*"); // which interfaces are considered, matches all interfaces by default
        string packetFilter = default("*"); // which packets are considered, matches all packets by default

        string icon = default("msg/packet_s"); // packet drop visualization icon
        double iconTintAmount = default(0); // colorization amount between 0 and 1, disabled by default
        string iconTintColor = default(""); // color for icon tinting, disabled by default

        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive packet drops fade out
        double fadeOutTime @unit(s) = default(1s); // how quickly packet drops fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are packet drops, value must be in the range (0, +inf)

        @class(PacketDropVisualizerBase);
}
