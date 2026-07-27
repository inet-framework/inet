//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.TagBase;

namespace inet;

//
// Request tag an application attaches to a TCP_C_SEND data packet (MSG_MORE) to
// indicate more data is coming: TCP holds this send's trailing sub-MSS partial
// segment (like TCP_CORK, but only for this one send) instead of transmitting it
// immediately. The held partial is flushed by a later send without MSG_MORE, an
// incoming ACK that opens the window, an explicit uncork/nodelay, or the cork
// (RTO/probe) timer. A SEND packet with no TcpSendMoreReq tag holds nothing back.
//
// @see ~TcpSocket::send
//
class TcpSendMoreReq extends TagBase
{
}
