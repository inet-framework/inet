//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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

package inet.networklayer.ipv4.ipsec;

//
// Implements basic IPsec (RFC 4301) functionality. It supports Authentication
// Header (AH) and Encapsulating Security Payload (ESP), in transport mode,
// for IPv4 unicast traffic (UDP/TCP/ICMP). A simple performance model to
// account for the overhead of cryptography is included. The IPsec databases
// SPD and SAD are stored in separate modules (~SecurityPolicyDatabase,
// ~SecurityAssociationDatabase).
//
// Limitations:
// - Does not perform actual cryptography. This was an explicit non-goal, as the
//   purpose of the model is to study the effects of the network overhead of IPsec.
//   As a consequence, encryption keys, certificates, and other cryptography-related
//   data do not appear in the model's data structures.
// - Key exchange protocols are not implemented. Instead, security associations (SAs)
//   are statically configured and remain in effect for the entire duration of the
//   simulation.
// - Transport mode only. Tunnel mode is not supported.
// - Multicast traffic is not supported. (Any multicast packet is let through unchanged.)
// - Anti-replay mechanism is not implemented.
// - DSCP-based SA selection is not implemented.
//
//
// <b>Configuration:</b>
//
// Security policies and security associations are defined using XML. The XML
// element specified in the spdConfig parameter should contain one or more
// SecurityPolicy elements. SecurityPolicy elements contain packet filtering
// elements (Selector, Direction), elements that define the action to be taken
// for matching packets (Action, Protection), and the list of SAs created
// for that policy (SecurityAssociation).
//
// Note that the order in which security policies and SAs appear in the
// configuration is significant, as they are searched in first-to-last
// order, and the first matching entry is used.
//
// Structural elements (notation: ?=optional, *=zero or more):
//
// - root: SecurityPolicy*
// - SecurityPolicy: Selector, Direction, Action, Protection?, EspMode?,
//   EncryptionAlg?, AuthenticationAlg?, MaxTfcPadLength?, SecurityAssociation*
// - Selector: LocalAddress?, RemoteAddress?, Protocol?, LocalPort?, RemotePort?, ICMPType?, ICMPCode?
// - SecurityAssociation: SPI, Selector?
//
// Contained elements may occur in any order.
//
// The following elements contain text content as value:
// - Direction: IN or OUT
// - Action: BYPASS, DISCARD, or PROTECT
// - Protection: AH or ESP. This element is only valid if Action is PROTECT.
// - EspMode: INTEGRITY, CONFIDENTIALITY, COMBINED. Only valid if Protection is ESP.
// - EncryptionAlg: Name of the encryption algorithm, only valid for Protection=ESP.
//   Possible values are: AES_CBC_128, AES_CBC_192, AES_CBC_256,
//   AES_GCM_16_128, AES_GCM_16_192, AES_GCM_16_256,
//   AES_CCM_8_128, AES_CCM_8_192, AES_CCM_8_256,
//   AES_CCM_16_128, AES_CCM_16_192, AES_CCM_16_256,
//   CHACHA20_POLY1305
// - AuthenticationAlg: Name of the authentication algorithm, only valid if Action is PROTECT,
//   and authentication is in use (protection is AH, or ESP with mode INTEGRITY or COMBINED).
//   Possible values are: NONE, HMAC_MD5_96, HMAC_SHA1, AES_128_GMAC, AES_192_GMAC,
//   AES_256_GMAC, HMAC_SHA2_256_128, HMAC_SHA2_384_192, HMAC_SHA2_512_256.
// - MaxTfcPadLength: Maximum length of random Traffic Confidentiality (TFC) padding.
//   The actual TFC pad length will be drawn with uniform distribution over [0,max].
// - LocalAddress, RemoteAddress: comma-separated list of addresses / address ranges
// - Protocol: comma-separated list of protocol names/numbers and/or their ranges.
//   Recognized names are TCP, UDP, and ICMP.
// - LocalPort, RemotePort: comma-separated list of port numbers / port number ranges
// - ICMPType, ICMPCode: comma-separated list of integers / integer ranges
//
// Ranges use "-" as a separator, and both the start and end values are inclusive.
// Example: "4-6" accepts 4, 5, and 6.
//
// Addresses are accepted in the standard dotted notation (10.0.0.5), and in any
// notation accepted by INET's L3AddressResolver, including module names ("host3")
// and module+interface ("host3%eth0"). Example: "host1, 10.0.0.1 - 10.0.0.15".
//
// As a rule, an SA inherits its properties from its parent policy. However,
// the selector can be changed/refined. Elements within SecurityAssociation's
// Selector override the corresponding elements specified in the parent
// SecurityPolicy's Selector element.
//
// Note that an SA needs to be configured at both endpoints to work.
// (Flip Direction and swap LocalAddress/RemoteAddress and LocalPort/RemotePort
// in the SecurityPolicy/SecurityAssociation for the other endpoint.)
// Also, since an SA defines a simplex channel, you'll need two SAs (2x2=4
// SecurityAssociation elements) to define a duplex channel.
//
// <pre>
//    <SecurityPolicy>
//        <Selector>
//            <LocalAddress>client1</LocalAddress>
//            <RemoteAddress>server3,10.0.0.1-10.0.0.15</RemoteAddress>
//            <Protocol>TCP</Protocol>
//            <LocalPort>1000</LocalPort>
//            <RemotePort>1000-1099,1200</RemotePort>
//        </Selector>
//        <Direction>IN</Direction>
//        <Action>PROTECT</Action>
//        <Protection>ESP</Protection>
//        <IcvNumBits>256</IcvNumBits>
//        <SecurityAssociation>
//            <SPI>14</SPI>
//            <Selector>
//                <RemotePort>1025</RemotePort>
//            </Selector>
//        </SecurityAssociation>
//    </SecurityPolicy>
// </pre>
//
// <b>Operation</b>
//
// For egress packets, security policies are searched in the order they
// were defined for the first match (with Direction=OUT and Selector matching
// the packet). If there is no match, the packet is discarded.
//
// Otherwise, the packet is processed according to the Action field. If Action is
// BYPASS, the packet is unaffected by IPsec. If Action is DROP, the packet is discarded.
//
// If Action is PROTECT, the list of SAs defined for the security policy is
// searched for matching ones (where Selector matches the packet). The protection
// defined in the SA is applied to the packet. (If there are multiple matching SAs,
// all of them are.) If Protection is AH, the packet is protected with an AH
// header. If Protection is ESP, the packet is protected with an ESP header. If
// Protection is AH_ESP, both headers are added. (Note that in an actual IPsec
// implementation, this would involve cryptographic processing, but the model does
// not contain actual cryptography.)
//
// For ingress packets, processing depends on whether the transport protocol in
// the packet is AH/ESP or something else.
//
// If it is AH or ESP, security associations are searched for an SA with Direction=IN
// and SPI matching the packet's SPI. If there is no matching SA, the packet
// is discarded. If there is a matching SA but the packet's protocol doesn't
// match the SA's Protection field, the packet is discarded. Otherwise, the
// AH or ESP header is removed from the packet.
//
// If the packet's protocol was not AH or ESP, the list of security policies
// is searched in order for the first matching entry. If there is no match,
// the packet is discarded. Otherwise, the Action field in the security policy
// defines what happens with the packet. If Action is DROP or BYPASS, the packet
// is processed accordingly. Action=PROTECT is considered invalid here, and thus
// the packet is discarded.
//
// <b>Performance model</b>
//
// The model contains a performance model that can be used to account for the finite
// runtime cost of cryptography. There are separate delays for IN/OUT and AH/ESP traffic;
// four in total. There are two queues, one for each direction. Only one packet can
// be processed at a time in either direction. If a second packet arrives during
// the IPsec protection, the packet is queued and reinjected later into the IPv4 layer.
//
// <b>Implementation</b>
//
// The IPsec module extends the IPv4 module via netfilter hooks. Egress traffic
// is processed in the POST_ROUTING hook, and ingress traffic in the LOCAL_IN
// hook.
//
// @see ~SecurityPolicyDatabase, ~SecurityAssociationDatabase
//
simple IPsec
{
    parameters:
		@display("i=block/control;is=s");

        string networkProtocolModule = default("^.ip");
        string interfaceTableModule;
		string spdModule = default("^.spd");
		string sadModule = default("^.sad");
		xml spdConfig;

		string defaultProtection = default("");  // If not "": value to use where <Protection> element is absent from the configuration
		string defaultEspMode = default(""); // If not "": value to use where <EspMode> element is absent from the configuration
		string defaultEncryptionAlg = default(""); // If not "": value to use where <EncryptionAlg> element is absent from the configuration
		string defaultAuthenticationAlg = default(""); // If not "": value to use where <defaultAuthenticationAlg> element is absent from the configuration
		int defaultMaxTfcPadLength = default(0); // Value to use where <MaxTfcPadLength> element is absent from the configuration

        volatile double ahProtectOutDelay @unit(s) = default(0.0s);
        volatile double ahProtectInDelay @unit(s) = default(0.0s);
        volatile double espProtectOutDelay @unit(s) = default(0.0s);
        volatile double espProtectInDelay @unit(s) = default(0.0s);

        @signal[inProtectedAccept](type=long);
        @signal[inProtectedDrop](type=long);
        @signal[inUnprotectedBypass](type=long);
        @signal[inUnprotectedDrop](type=long);
        @signal[outBypass](type=long);
        @signal[outProtect](type=long);
        @signal[outDrop](type=long);
        @signal[inProcessDelay](type=double);
        @signal[outProcessDelay](type=double);

        @statistic[inProtectedAccept](title="Incoming Protected Packet Accepted"; record=count,vector);
        @statistic[inProtectedDrop](title="Incoming Protected Packet Dropped"; record=count,vector);
        @statistic[inUnprotectedBypass](title="Incoming Unprotected Packet Bypassed"; record=count,vector);
        @statistic[inUnprotectedDrop](title="Incoming Unprotected Packet Dropped"; record=count,vector);
        @statistic[outBypass](title="Outgoing Packet Bypassed"; record=count,vector);
        @statistic[outProtect](title="Outgoing Packet Protected"; record=count,vector);
        @statistic[outDrop](title="Outgoing Packet Dropped"; record=count,vector);
        @statistic[inProcessDelay](title="Incoming Processing Delay"; record=vector,stats);
        @statistic[outProcessDelay](title="Outgoing Processing Delay"; record=vector,stats);

}
