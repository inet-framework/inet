//
// Copyright (C) 2008 by Irene Ruengeler
// Copyright (C) 2012 by Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPCOMMAND_H
#define __INET_SCTPCOMMAND_H

#include "inet/common/INETDefs.h"

namespace inet {

//
// SCTP command codes, sent by the application to SCTP. These constants
// should be set as message kind on a message sent to the SCTP entity.
//
// @see SctpCommand, SctpOpenCommand, Sctp
//
enum SctpCommandCode {
    SCTP_C_ASSOCIATE = 1,    // active open (must carry SctpOpenCommand)
    SCTP_C_OPEN_PASSIVE = 2,    // passive open (must carry SctpOpenCommand)
    SCTP_C_SEND = 3,    // send data (set on data packet)
    SCTP_C_CLOSE = 5,    // shutdown the association
    SCTP_C_ABORT = 6,    // abort connection
    SCTP_C_STATUS = 7,    // request status info (SCTP_I_STATUS) from Sctp
    SCTP_C_RECEIVE = 8,    // data receive request
    SCTP_C_SEND_ORDERED = 9,    // send data ordered
    SCTP_C_SEND_UNORDERED = 10,    // send data unordered
    SCTP_C_PRIMARY = 11,    // set primary path
    SCTP_C_QUEUE_BYTES_LIMIT = 12,    // set send queue limit (in bytes)
    SCTP_C_QUEUE_MSGS_LIMIT = 13,    // set send queue limit (in messages)
    SCTP_C_SHUTDOWN = 14,
    SCTP_C_NO_OUTSTANDING = 15,
    SCTP_C_STREAM_RESET = 16,    // send StreamResetChunk
    SCTP_C_RESET_ASSOC = 17,
    SCTP_C_ADD_STREAMS = 18,
    SCTP_C_NAT_INFO = 19,
    SCTP_C_SEND_ASCONF = 20,
    SCTP_C_SET_STREAM_PRIO = 21,
    SCTP_C_GETSOCKETOPTIONS = 22,
    SCTP_C_ACCEPT = 23,
    SCTP_C_SET_RTO_INFO = 24,
    SCTP_C_ACCEPT_SOCKET_ID = 25,
    SCTP_C_DESTROY = 26    // destroy socket
};

//
// SCTP indications, sent by SCTP to the application. SCTP will set these
// constants as message kind on messages it sends to the application.
//
// @see SctpCommand, SctpStatusInfo, Sctp
//
enum SctpStatusInd {
    SCTP_I_DATA = 1,    // data packet (set on data packet)
    SCTP_I_DATA_NOTIFICATION = 2,    // data arrived notification
    SCTP_I_ESTABLISHED = 3,    // connection established
    SCTP_I_PEER_CLOSED = 4,    // FIN received from remote SCTP
    SCTP_I_CLOSED = 5,    // connection closed normally (via FIN exchange)
    SCTP_I_CONNECTION_REFUSED = 6,    // connection refused
    SCTP_I_CONNECTION_RESET = 7,    // connection reset
    SCTP_I_TIMED_OUT = 8,    // conn-estab timer went off, or max retransm. count reached
    SCTP_I_STATUS = 9,    // status info (will carry SctpStatusInfo)
    SCTP_I_ABORT = 10,    // association was aborted by the peer
    SCTP_I_CONN_LOST = 11,    // association had too many retransmissions FIXME
    SCTP_I_SEND_MSG = 12,
    SCTP_I_SHUTDOWN_RECEIVED = 13,
    SCTP_I_SENDQUEUE_FULL = 14,
    SCTP_I_SENDQUEUE_ABATED = 15,
    SCTP_I_ABANDONED = 16,
    SCTP_I_SEND_STREAMS_RESETTED = 17,
    SCTP_I_RCV_STREAMS_RESETTED = 18,
    SCTP_I_RESET_REQUEST_FAILED = 19,
    SCTP_I_ADDRESS_ADDED = 20,    // used for AddIP and multihomed NAT
    SCTP_I_SENDSOCKETOPTIONS = 21,
    SCTP_I_AVAILABLE = 22
};

enum SctpFlags {
    COMPLETE_MESG_UNORDERED = 1,
    COMPLETE_MESG_ORDERED = 0
};

} // namespace inet

#endif // ifndef __INET_SCTPCOMMAND_H

