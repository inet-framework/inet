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

package inet.emulation.linklayer.ethernet;

//
// This module provides a bidirectional connection to a real TAP device of the
// host computer which is running the simulation. It writes the packets arrived
// on 'lowerLayerIn' gate to the specified real TAP device, and sends out packets
// arrived from real TAP device on 'lowerLayerOut' gate.
//
simple ExtEthernetTapDevice
{
    parameters:
        string device; // name of existing real tap interface
        string namespace = default("");
        string packetNameFormat = default("Ext-%p-%t-%n");
        @display("i=block/rxtx");
        @signal[packetSentToLower](type=Packet);
        @signal[packetReceivedFromLower](type=Packet);
        @signal[packetSent](type=Packet);
        @signal[packetReceived](type=Packet);
        @statistic[packetSentFromLower](title="packets sent to higher layer"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceivedFromLower](title="packets received from higher layer"; source=packetReceivedFromLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

