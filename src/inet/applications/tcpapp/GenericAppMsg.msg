//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.Units;
import inet.common.packet.chunk.Chunk;

namespace inet;

//
// Message class for generic request-reply style applications.
// The client sends a ~GenericAppMsg which contains the number of
// bytes the client wants the server to send back as reply.
//
// This way intelligence (behaviour specific to the modelled application,
// e.g. HTTP, SMB, database protocol) needs only to be present in
// the client, and the server model can be kept simple and dumb.
//
// @see ~TcpGenericServerApp, TCPGenericCliAppBase (C++ only)
//
class GenericAppMsg extends FieldsChunk
{
    B expectedReplyLength;   // in bytes
    double replyDelay;       // reply after this many seconds
    bool serverClose;        // with TCP: if true, server should close the
                             // connection after sending the reply
}
