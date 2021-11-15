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
// This module subscribes for packetDropped on the module determined by the
// visualizationSubjectModule parameter. If the visualizer receives a packet dropped
// signal, then it stores the dropped packet and starts the visualization.
//
// The format string can contain the following directives:
//  - %n packet name
//  - %c packet class
//  - %r drop reason
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~IPacketDropVisualizer, ~VisualizerBase
//
simple PacketDropVisualizerBase extends VisualizerBase
{
    parameters:
        bool displayPacketDrops = default(false); // display packet drops, disabled by default

        string nodeFilter @mutable = default("*"); // which network nodes are considered, matches all network nodes by default
        string interfaceFilter @mutable = default("*"); // which network interfaces are considered, matches all network interfaces by default
        string packetFilter @mutable = default("*"); // which packets are considered, matches all packets by default
        string packetDataFilter @mutable = default("*"); // which packets are considered based on the data they contain, matches all packets by default
        string detailsFilter @mutable = default("*"); // which packet drops are considered, matches all packet drops by default

        string icon = default("msg/packet_s"); // packet drop visualization icon
        double iconTintAmount = default(1); // colorization amount between 0 and 1, enabled by default
        string iconTintColor = default("dark"); // icon tint color is a list of colors, a set of dark colors by default

        string labelFormat @mutable = default("%n"); // determines what data is displayed in the label, packet name by default
        string labelFont = default("<default>"); // label font, automatic by default
        string labelColor = default("black"); // label color, black by default

        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime"); // specifies how inactive packet drops fade out, real time by default
        double fadeOutTime @unit(s) = default(1s); // how quickly packet drops fade away, 1 second by default
        double fadeOutAnimationSpeed = default(0); // animation speed while there are packet drops, value must be in the range (0, +inf), disabled by default

        @class(PacketDropVisualizerBase);
}

