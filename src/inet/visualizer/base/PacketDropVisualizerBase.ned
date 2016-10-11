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
// This module subscribes for packetFromLowerDroppedSignal and
// packetFromUpperDroppedSignal on the module determined by the subscriptionModule
// parameter. If the visualizer receives a packet dropped signal, then it stores
// the dropped packet and starts the drop visualization. This module also
// subscribes for mobilityStateChangedSignal to be able to follow the movement
// of mobile nodes.
//
// @see ~PacketDropCanvasVisualizer, ~PacketDropOsgVisualizer, ~IPacketDropVisualizer, ~PacketDropVisualizerBase
//
simple PacketDropVisualizerBase extends VisualizerBase
{
    parameters:
        string subscriptionModule = default(""); // module path where the visualizer subscribes for packet drop signals, root module by default
        string packetNameFilter = default(""); // which packets are considered, matches no packets by default
        string icon = default("msg/packet_s"); // packet drop visualizationicon
        double iconTintAmount = default(0); // colorization amount between 0 and 1, disabled by default
        string iconTintColor = default(""); // color for icon tinting, disabled by default
        string fadeOutMode @enum("realTime", "animationTime", "simulationTime") = default("realTime");
        double fadeOutHalfLife @unit(s) = default(1s); // how quickly packet drops fade away, opacity halves every second by default
        @class(PacketDropVisualizerBase);
}
