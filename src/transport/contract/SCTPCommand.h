//
// Copyright (C) 2008 by Irene Ruengeler
// Copyright (C) 2010 by Thomas Dreibholz
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

#ifndef __SCTPCOMMAND_H
#define __SCTPCOMMAND_H


//
// SCTP command codes, sent by the application to SCTP. These constants
// should be set as message kind on a message sent to the SCTP entity.
//
// @see SCTPCommand, SCTPOpenCommand, SCTP
//
enum SctpCommandCode
{
    SCTP_C_ASSOCIATE            = 1,        // active open (must carry SCTPOpenCommand)
    SCTP_C_OPEN_PASSIVE         = 2,        // passive open (must carry SCTPOpenCommand)
    SCTP_C_SEND                 = 3,        // send data (set on data packet)
    SCTP_C_CLOSE                = 5,        // shutdown the association
    SCTP_C_ABORT                = 6,        // abort connection
    SCTP_C_STATUS               = 7,        // request status info (SCTP_I_STATUS) from SCTP
    SCTP_C_RECEIVE              = 8,        // data receive request
    SCTP_C_SEND_ORDERED         = 9,        // send data ordered
    SCTP_C_SEND_UNORDERED       = 10,   // send data unordered
    SCTP_C_PRIMARY              = 11,   // set primary path
    SCTP_C_QUEUE_BYTES_LIMIT    = 12,   // set send queue limit (in bytes)
    SCTP_C_QUEUE_MSGS_LIMIT     = 13,   // set send queue limit (in messages)
    SCTP_C_SHUTDOWN             = 14,
    SCTP_C_NO_OUTSTANDING       = 15
};

//
// SCTP indications, sent by SCTP to the application. SCTP will set these
// constants as message kind on messages it sends to the application.
//
// @see SCTPCommand, SCTPStatusInfo, SCTP
//
enum SctpStatusInd
{
    SCTP_I_DATA                 = 1,                     // data packet (set on data packet)
    SCTP_I_DATA_NOTIFICATION    = 2,     // data arrived notification
    SCTP_I_ESTABLISHED          = 3,             // connection established
    SCTP_I_PEER_CLOSED          = 4,             // FIN received from remote SCTP
    SCTP_I_CLOSED               = 5,                 // connection closed normally (via FIN exchange)
    SCTP_I_CONNECTION_REFUSED   = 6, // connection refused
    SCTP_I_CONNECTION_RESET     = 7,     // connection reset
    SCTP_I_TIMED_OUT            = 8,             // conn-estab timer went off, or max retransm. count reached
    SCTP_I_STATUS               = 9,                 // status info (will carry SCTPStatusInfo)
    SCTP_I_ABORT                = 10,                // association was aborted by the peer
    SCTP_I_CONN_LOST            = 11,            // association had too many retransmissions FIXME
    SCTP_I_SEND_MSG             = 12,
    SCTP_I_SHUTDOWN_RECEIVED    = 13,
    SCTP_I_SENDQUEUE_FULL       = 14,
    SCTP_I_SENDQUEUE_ABATED     = 15
};


//
// Currently not in use.
//

#endif


