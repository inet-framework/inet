//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.test.moduletest.lib.ieee80211retransmissiontest;

import inet.applications.contract.IApp;

//
// A very simple MPDU generator module
//
simple MpduGen like IApp
{
    // List of packets genereted by this module.
    // S means short data packet (SendDataWithACKFrame)
    // L means long data packet that is preceded by an RTS frame (SendDataWithRtsCts)
    // Example: packets = "SLS"
    string packets;
    string packetName = default("MPDU");
    int longPacketSize @unit(B) = default(1700B);
    int shortPacketSize @unit(B) = default(500B);

    string interfaceTableModule;   // The path to the InterfaceTable module
    int localPort = default(-1);  // local port (-1: use ephemeral port)
    int destPort;
    string destAddress;
    string localAddress = default("");
    double startTime @unit(s) = default(0.05s);
    @display("i=block/app");
    @signal[packetSent](type=cPacket);
    @signal[packetReceived](type=cPacket);
    @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
