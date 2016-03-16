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
#include "PacketDrillApp.h"

namespace inet {

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
