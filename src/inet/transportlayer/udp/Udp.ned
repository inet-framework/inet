//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.transportlayer.udp;

import inet.common.SimpleModule;
import inet.transportlayer.contract.IUdp;

//
// UDP protocol implementation, for IPv4 (~Ipv4) and IPv6 (~Ipv6).
//
// The UDP protocol header is represented by the class ~UdpHeader.
//
// <b>Communication with clients (applications)</b>
//
// The module can (should) be connected to several applications.
// For sending a UDP packet, the application should attach an ~UdpCommand
// object to the payload and send it to ~Udp.
// ~Udp will also attach an ~UdpCommand object to any payload
// message it sends up to the application.
//
// For receiving UDP packets, the connected applications should first
// "bind" to the given UDP port. This can be done by sending an
// arbitrary message with message kind UDP_C_BIND and an ~UdpCommand
// attached with `srcPort` filled in.
//
// When UDP receives an ICMP error (~IcmpHeader or ~Icmpv6Header)
// that refers to a UDP socket, it reports the error to the corresponding
// application by sending a message with kind UDP_I_ERROR.
//
// <b>Communication with the IP (IPv4/IPv6) layer</b>
//
// The ~Udp model relies on sending and receiving ~L3AddressReq/~L3AddressInd
// tags attached to UDP packets.
//
// Checksum operation modes for outgoing UDP packets (`checksumMode` parameter in UDP module):
//  - "disabled" means the outgoing packet will have its checksum set to 0 (serializable)
//  - "declared" means the outgoing packet will have the correct checksum declared
//     without actually computing it; the value is set to `0xC00D` (not serializable)
//  - "computed" means the outgoing packet will have the correctly computed checksum
//     as defined by the RFC (serializable)
//
// Checksum operation modes for incoming UDP packets (`checksumMode` field in ~UdpHeader chunk):
//  - CHECKSUM_DISABLED means the incoming packet must have its checksum set to 0 (serializable)
//  - CHECKSUM_DECLARED_CORRECT means the incoming packet is assumed to have a
//    correct checksum without the actual value (not serializable)
//  - CHECKSUM_DECLARED_INCORRECT means the incoming packet is assumed to have an
//    incorrect checksum without the actual value (not serializable)
//  - CHECKSUM_COMPUTED means the incoming packet has a potentially incorrect checksum
//    that is to be verified as defined by the RFC (serializable)
// The incoming packet also fails the checksum verification if any relevant chunk is marked incorrect.
//
// @see ~UdpHeader, ~UdpCommand, ~UdpCommandCode
//
simple Udp extends SimpleModule like IUdp
{
    parameters:
        @class(Udp);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string icmpModule = default("^.ipv4.icmp");
        string icmpv6Module = default("^.ipv6.icmpv6");
        string checksumMode @enum("disabled", "declared", "computed") = default("declared");
        bool defaultMulticastLoop = default(true);
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
        @selfMessageKinds();    // Kind not used in self messages or model doesn't use any self message
        @directSendKinds();     // Kind not used in sendDirect() messages or model doesn't use any sendDirect()

    gates:
        input appIn @labels(UdpCommand/down) @messageKinds(inet::UdpCommandCode);
        input ipIn @labels(UdpHeader,Ipv4ControlInfo/up,Ipv6ControlInfo/up);
        output appOut @labels(UdpCommand/up) @messageKinds(inet::UdpStatusInd);
        output ipOut @labels(UdpHeader,Ipv4ControlInfo/down,Ipv6ControlInfo/down);
}

simple UdpChecksumInsertionHook extends SimpleModule
{
    parameters:
        @class(UdpChecksumInsertionHook);
}

