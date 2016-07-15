//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/Protocol.h"
#include "inet/common/TagBase_m.h"
}}

namespace inet;

cplusplus{{
typedef const Protocol * ProtocolPtr;
}}

class noncobject ProtocolPtr;
class TagBase extends cObject;

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
class ProtocolTag extends ProtocolTagBase
{
}

//
// This tag specifies the transport protocol of the packet.
// It is present from the transport protocol to the physical layer.
//
class TransportProtocolTag extends ProtocolTagBase
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
// This indication specifies the transport protocol that was used to receive the packet.
// It is present from the transport protocol to the application.
//
class TransportProtocolInd extends ProtocolTagBase
{
}

//
// This tag specifies the network protocol of the packet.
// It is present from the network protocol to the physical layer.
//
class NetworkProtocolTag extends ProtocolTagBase
{
}

//
// This request determines the network protocol that should be used to send the packet.
// It may be present from the application to the network protocol.
//
class NetworkProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the network protocol that was used to receive the packet.
// It is present from the network protocol to the application.
//
class NetworkProtocolInd extends ProtocolTagBase
{
}

//
// This tag specifies the mac protocol of the packet.
// It is present from the mac protocol to the physical layer.
//
class MacProtocolTag extends ProtocolTagBase
{
}

//
// This request determines the mac protocol that should be used to send the packet.
// It may be present from the application to the mac protocol.
//
class MacProtocolReq extends ProtocolTagBase
{
}

//
// This indication specifies the mac protocol that was used to receive the packet.
// It is present from the mac protocol to the application.
//
class MacProtocolInd extends ProtocolTagBase
{
}
