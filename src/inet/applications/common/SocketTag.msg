//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus{{
#include "inet/common/TagBase_m.h"
}}

namespace inet;

class TagBase extends cObject;

class SocketTag extends TagBase
{
    int socketId = -1; // a locally unique (network node) identifier that determines the application socket
}

//
// Determines the application socket of an outgoing packet, sent by the application.
//
class SocketReq extends SocketTag
{
}

//
// Specifies the application socket of an incoming packet, received by the application.
//
class SocketInd extends SocketTag
{
}
