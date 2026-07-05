//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

//
// Region tag carried on the request data of generic request-reply style TCP
// applications (~TcpBasicClientApp, ~TelnetApp). It holds the per-request
// control that the (simulated) server needs: how many bytes to send back as
// reply, after what delay, and whether to close the connection afterwards.
//
// This way the intelligence (behaviour specific to the modelled application,
// e.g. HTTP, SMB, a database protocol) needs only to be present in the client,
// and the server model (~TcpGenericServerApp) can be kept simple and dumb.
//
// This is MODEL metadata, not real wire data. It is attached to the byte region
// of the (opaque) request payload, so it rides along end-to-end over any
// transport that preserves region tags -- the INET Tcp module and ordinary
// links, even when the packet is serialized on the wire. It is deliberately
// NOT serializable: over a byte-accurate transport (~TcpLwip, emulation) the
// data crosses as raw octets and the tag is lost, at which point
// ~TcpGenericServerApp raises an error -- request/reply control instructions
// have no on-the-wire representation and must not be faked into bytes.
//
class GenericAppMsgReq extends TagBase
{
    B requestLength;        // total length of the request; used to frame the received byte stream
    B expectedReplyLength;  // number of bytes the server should send back as reply
    double replyDelay;      // the server sends the reply this many seconds after receiving the request
    bool serverClose;       // if true, the server closes the connection after sending the reply
}
