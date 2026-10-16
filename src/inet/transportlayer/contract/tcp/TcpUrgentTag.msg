//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.TagBase;

namespace inet;

//
// Request tag an application attaches to a TCP_C_SEND data packet to send it as
// urgent data (RFC 9293 section 3.8.5, Linux MSG_OOB): the urgent pointer of TCP
// points to the octet after the data of this SEND (MUST-62), and the segments
// before it carry URG until the peer acknowledges them.
//
// @see ~TcpSocket::sendUrgent
//
class TcpSendUrgentReq extends TagBase
{
}
