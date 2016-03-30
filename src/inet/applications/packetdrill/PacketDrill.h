//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_PACKETDRILL_H
#define __INET_PACKETDRILL_H

#include "inet/common/INETDefs.h"

#include "PacketDrillUtils.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"
#include "inet/transportlayer/sctp/SCTPMessage.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "PacketDrillApp.h"

namespace inet {
using namespace sctp;
class PacketDrillApp;
}

class INET_API PacketDrill
{
    protected:
        static PacketDrillApp *pdapp;

    public:
        PacketDrill(PacketDrillApp* mod);
        ~PacketDrill();

        static cPacket *buildUDPPacket(int address_family, enum direction_t direction,
                                       uint16 udp_payload_bytes, char **error);

        static cPacket* buildTCPPacket(int address_family, enum direction_t direction,
                                       const char *flags, uint32 startSequence,
                                       uint16 tcpPayloadBytes, uint32 ackSequence,
                                       int32 window, cQueue *tcpOptions, char **error);

        static cPacket* buildSCTPPacket(int address_family, enum direction_t direction,
                                        cQueue *chunks);

        static PacketDrillSctpChunk* buildDataChunk(int64 flgs, int64 len, int64 tsn, int64 sid, int64 ssn, int64 ppid);

        static PacketDrillSctpChunk* buildInitChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters);

        static PacketDrillSctpChunk* buildInitAckChunk(int64 flgs, int64 tag, int64 a_rwnd, int64 os, int64 is, int64 tsn, cQueue *parameters);

        static PacketDrillSctpChunk* buildSackChunk(int64 flgs, int64 cum_tsn, int64 a_rwnd, cQueue *gaps, cQueue *dups);

        static PacketDrillSctpChunk* buildCookieEchoChunk(int64 flgs, int64 len, PacketDrillBytes *cookie);

        static PacketDrillSctpChunk* buildCookieAckChunk(int64 flgs);

        static PacketDrillSctpChunk* buildShutdownChunk(int64 flgs, int64 cum_tsn);

        static PacketDrillSctpChunk* buildShutdownAckChunk(int64 flgs);

        static PacketDrillSctpChunk* buildShutdownCompleteChunk(int64 flgs);

        static PacketDrillSctpChunk* buildAbortChunk(int64 flgs);

        static PacketDrillSctpChunk* buildHeartbeatChunk(int64 flgs, PacketDrillSctpParameter *info);

        static PacketDrillSctpChunk* buildHeartbeatAckChunk(int64 flgs, PacketDrillSctpParameter *info);

        static IPv4Datagram *makeIPPacket(int protocol, enum direction_t direction, L3Address localAddr, L3Address remoteAddr);

        int evaluateExpressionList(cQueue *in_list, cQueue *out_list, char **error);

        int evaluate(PacketDrillExpression *in, PacketDrillExpression *out, char **error);

        int evaluate_binary_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error);

        int evaluate_pollfd_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error);

        int evaluate_msghdr_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error);

        int evaluate_iovec_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error);

        int evaluateListExpression(PacketDrillExpression *in, PacketDrillExpression *out, char **error);
};

#endif
