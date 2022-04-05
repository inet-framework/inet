//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Protocol;
import inet.common.TagBase;
import inet.common.packet.chunk.Chunk;

namespace inet;

cplusplus{{
typedef Ptr<const Chunk> ChunkPtr;
}}

class ChunkPtr
{
    @existingClass;
    @descriptor(readonly);
    Chunk *content @getter(get);
}

//
// This is an abstract base class that should not be directly added as a tag.
//
class ProtocolTagBase extends TagBase
{
    const Protocol *protocol @getter(findProtocol); // specifies a protocol (internal class with an artificial identifier)
}

cplusplus(ProtocolTagBase) {{
    const Protocol *getProtocol() const {
        if (protocol == nullptr)
            throw cRuntimeError("Protocol is not specified");
        return protocol;
    }
}}

//
// This is an abstract base class that should not be directly added as a tag.
//
class ProtocolListTagBase extends TagBase
{
    const Protocol *protocol[]; // specifies a list of protocols (internal class with an artificial identifier)
}

//
// OSI layered service primitives
//
enum ServicePrimitive
{
    SP_UNSPECIFIED = -1;
    SP_REQUEST    = 1;
    SP_INDICATION = 2;
    SP_RESPONSE   = 3;
    SP_CONFIRM    = 4;
}

//
// This request determines the destination protocol of the message or packet
// when it's sent from one protocol module to another using the ~MessageDispatcher.
//
class DispatchProtocolReq extends ProtocolTagBase
{
    ServicePrimitive servicePrimitive;
}

//
// This indication specifies the sending protocol of the message or packet
// when it's sent from one protocol module to another using the ~MessageDispatcher.
//
class DispatchProtocolInd extends ProtocolTagBase
{
}

//
// This request determines the expected protocol encapsulation order of the
// packet. If this tag is attached to a packet, then the first encapsulation
// protocol specifies the next dispatch protocol. Otherwise the individual
// protocol modules are expected to request dispatching to their default
// following encapsulation protocol.
//
class EncapsulationProtocolReq extends ProtocolListTagBase
{
}

//
// This indication determines the original protocol encapsulation order of the
// packet. If this tag is attached to the packet, then the protocols specify the
// order of encapsulation as it seen by the protocol modules when the packet was
// processed and decapsulated.
//
class EncapsulationProtocolInd extends ProtocolListTagBase
{
}

//
// This tag specifies the protocol of the packet.
//
// Packet processing at the sender
// ===============================
//
// | ApplicationData |
//     whole packet has no protocol
//     data part has no protocol
//
// | UdpHeader ApplicationData |
//     whole packet has UDP protocol
//     data part has UDP protocol
//
// | Ipv4Header UdpHeader ApplicationData |
//     whole packet has IPv4 protocol
//     data part has IPv4 protocol
//
// EthernetMacHeader | Ipv4Header UdpHeader ApplicationData |
//     whole packet has incomplete Ethernet protocol
//     data part has IPv4 protocol
//
// EthernetMacHeader | Ipv4Header UdpHeader ApplicationData | EthernetPadding EthernetFcs
//     whole packet has Ethernet protocol
//     data part has IPv4 protocol
//
// | EthernetMacHeader Ipv4Header UdpHeader ApplicationData EthernetPadding EthernetFcs |
//     whole packet has Ethernet protocol
//     data part has Ethernet protocol
//
// Packet processing at the receiver
// =================================
//
// | EthernetMacHeader Ipv4Header UdpHeader ApplicationData EthernetPadding EthernetFcs |
//     whole packet has Ethernet protocol
//     data part has Ethernet protocol
//
// EthernetMacHeader Ipv4Header UdpHeader ApplicationData EthernetPadding | EthernetFcs
//     whole packet has Ethernet protocol
//     data part has no protocol
//
// EthernetMacHeader | Ipv4Header UdpHeader ApplicationData | EthernetPadding EthernetFcs
//     whole packet has Ethernet protocol
//     data part has IPv4 protocol
//
// EthernetMacHeader Ipv4Header | UdpHeader ApplicationData | EthernetPadding EthernetFcs
//     whole packet has Ethernet protocol
//     data part has UDP protocol
//
// EthernetMacHeader Ipv4Header UdpHeader | ApplicationData | EthernetPadding EthernetFcs
//     whole packet has Ethernet protocol
//     data part has no protocol
//
class PacketProtocolTag extends ProtocolTagBase
{
    b frontOffset = b(0); // extra offset relative to the packet data part front offset
    b backOffset = b(0); // extra offset relative to the packet data part back offset
}

cplusplus(PacketProtocolTag) {{
    void set(const Protocol *protocol, b frontOffset = b(0), b backOffset = b(0)) {
        this->protocol = protocol;
        this->frontOffset = frontOffset;
        this->backOffset = backOffset;
    }
}}

//
// This request determines the transport protocol that should be used to send the packet.
// It may be present from the application to the transport protocol.
//
class TransportProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the transport protocol that was used to receive
// the packet. It may be present from the transport protocol to the application
// and from the transport protocol to the physical layer.
//
class TransportProtocolInd extends ProtocolTagBase
{
    ChunkPtr transportProtocolHeader;
}

//
// This request determines the network protocol that should be used to send the
// packet. It may be present from the application to the network protocol and
// from the network protocol to the physical layer.
//
class NetworkProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the network protocol that was used to receive the
// packet. It may be present from the network protocol to the application.
//
class NetworkProtocolInd extends ProtocolTagBase
{
    ChunkPtr networkProtocolHeader;
}

//
// This request determines the mac protocol that should be used to send the
// packet. It may be present from the application to the mac protocol and from
// the mac protocol to the physical layer.
//
class MacProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the mac protocol that was used to receive the
// packet. It may be present from the mac protocol to the application.
//
class MacProtocolInd extends ProtocolTagBase
{
    ChunkPtr macProtocolHeader;
}
