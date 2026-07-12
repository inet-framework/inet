//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.TagBase;

namespace inet;

//
// Request tag an application attaches to a TCP_C_SEND data packet (Workstream
// H1, MSG_EOR) to mark the packet's last byte as a record boundary: TCP will
// not build a segment spanning it, i.e. it will never coalesce this SEND's
// data together with a later SEND's data into the same outgoing segment. A
// SEND packet with no TcpSendEorReq tag behaves exactly as before this
// feature existed.
//
// @see ~TcpSocket::send
//
class TcpSendEorReq extends TagBase
{
}
