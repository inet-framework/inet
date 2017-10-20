//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;

cplusplus{{
#include "inet/common/Protocol.h"
#include "inet/common/Units.h"      //FIXME Why? Units unused in this file.
#include "inet/common/packet/chunk/Chunk.h"
}}

import inet.common.TagBase;

namespace inet;

cplusplus{{
typedef Ptr<const Chunk> ChunkPtr;
typedef const Protocol * ProtocolPtr;

using namespace units::values;      //FIXME Why? Units unused in this file.
}}

class noncobject ChunkPtr;
class noncobject ProtocolPtr;

//
// This is an abstract base class that should not be directly added as a tag.
//
class ProtocolTagBase extends TagBase
{
    ProtocolPtr protocol @tostring("->info()") = nullptr; // specifies a protocol (internal class with an artificial identifier)
}

//
// This request determines the destination protocol of the message or packet
// when it's sent from one protocol module to another using the ~MessageDispatcher.
//
class DispatchProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the sending protocol of the message or packet
// when it's sent from one protocol module to another using the ~MessageDispatcher.
//
class DispatchProtocolInd extends ProtocolTagBase
{
}

//
// This tag specifies the protocol of the packet.
//
class PacketProtocolTag extends ProtocolTagBase
{
}

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
    ChunkPtr transportProtocolHeader = nullptr;
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
    ChunkPtr networkProtocolHeader = nullptr;
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
    ChunkPtr macProtocolHeader = nullptr;
}
