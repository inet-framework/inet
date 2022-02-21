//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
