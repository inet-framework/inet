//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
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

package inet.transportlayer.udp;

import inet.transportlayer.contract.IUdp;

//
// UDP protocol implementation, for IPv4 (~Ipv4) and IPv6 (~Ipv6).
//
// The UDP protocol header is represented by the class ~UdpHeader.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending an UDP packet, the application should attach an ~UdpControlInfo
// object to the payload, and send it to ~Udp.
// ~Udp will also attach an ~UdpControlInfo object to any payload
// message in sends up to the application.
//
// For receiving Udp packets, the connected applications should first
// "bind" to the given UDP port. This can be done by sending an
// arbitrary message with message kind UDP_C_BIND and an ~UdpControlInfo
// attached with srcPort filled in.
//
// When UDP receives an ICMP error (~IcmpHeader or ~Icmpv6Header)
// that refers to an UDP socket, it reports the error to the corresponding
// application by sending a message with kind UDP_I_ERROR.
//
// <b>Communication with the IP (IPv4/IPv6) layer</b>
//
// The ~Udp model relies on sending and receiving ~L3AddressReq/~L3AddressInd
// tags attached to UDP packets.
//
// CRC operation modes for outgoing UDP packets (crcMode parameter in UDP module):
//  - "disabled" means the outgoing packet will have its CRC set to 0 (serializable)
//  - "declared" means the outgoing packet will have the correct CRC declared
//     without actually computing it, the value is set to 0xC00D (not serializable)
//  - "computed" means the outgoing packet will have the correctly computed CRC
//     as defined by the RFC (serializable)
// CRC operation modes for incoming UDP packets (crcMode field in ~UdpHeader chunk):
//  - CRC_DISABLED means the incoming packet must have its CRC set to 0 (serializable)
//  - CRC_DECLARED_CORRECT means the incoming packet is assumed to have a
//    correct CRC without the actual value (not serializable)
//  - CRC_DECLARED_INCORRECT means the incoming packet is assumed to have an
//    incorrect CRC without the actual value (not serializable)
//  - CRC_COMPUTED means the incoming packet has a potentially incorrect CRC
//    that is to be verified as defined by the RFC (serializable)
// The incoming packet also fails the CRC verification if any relevant chunk is marked incorrect.
//
// @see ~UdpHeader, ~UdpControlInfo, ~UdpCommandCode
//
simple Udp like IUdp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string icmpModule = default("^.ipv4.icmp");
        string icmpv6Module = default("^.ipv6.icmpv6");
        string crcMode @enum("disabled", "declared", "computed") = default("declared");
        @display("i=block/transport");
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[packetSentToLower](type=cPacket);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetDropped](type=inet::Packet);
        @signal[packetSent](type=cPacket);
        @signal[packetReceived](type=cPacket);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed up"; source=packetSentToUpper; record=count,"vector(count)"; interpolationmode=none);
        @statistic[droppedPkWrongPort](title="packets dropped (wrong port)"; source=packetDropReasonIsNoPortFound(packetDropped); record=count,"vector(count)"; interpolationmode=none);
        @statistic[droppedPkBadChecksum](title="packets dropped (checksum error)"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,"vector(count)"; interpolationmode=none);
        @selfMessageKinds();    // kind not used in self messages or model doesn't use any self message
        @directSendKinds();     // kind not used in sendDirect() messages or model doesn't use any sendDirect()

    gates:
        input appIn @labels(UdpControlInfo/down) @messageKinds(inet::UdpCommandCode);
        input ipIn @labels(UdpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(UdpControlInfo/up) @messageKinds(inet::UdpStatusInd);
        output ipOut @labels(UdpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

