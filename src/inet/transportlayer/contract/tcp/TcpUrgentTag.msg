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

//
// Indication tag on a TCP_I_URGENT_DATA packet, data before the urgent mark: the
// urgent bytes that remain after this packet (RFC 9293 section 3.8.5, MUST-33, and
// the URGENT flag of the RECEIVE call). A packet never holds data after the mark.
//
class TcpUrgentInd extends TagBase
{
    uint32_t remaining; // the urgent bytes after this packet; 0: the urgent data ends here
}

//
// Indication tag on a TCP_I_URGENT_DATA packet of one byte: the last urgent byte,
// which the urgentOutOfBand parameter of Tcp takes out of the stream, as Linux
// (MSG_OOB without SO_OOBINLINE). A later urgent pointer gives a new byte.
//
class TcpOutOfBandInd extends TagBase
{
}
