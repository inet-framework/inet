//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

class SocketTagBase extends TagBase
{
    int socketId = -1; // a locally unique (network node) identifier that determines the application socket
}

//
// Determines the application socket of an outgoing packet, sent by the application.
//
class SocketReq extends SocketTagBase
{
}

//
// Specifies the application socket of an incoming packet, received by the application.
//
class SocketInd extends SocketTagBase
{
}
