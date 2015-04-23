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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "inet/applications/tunapp/PacketDrill.h"
#include "inet/applications/tunapp/PDUtils.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/transportlayer/tcp_common/TCPSegment_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"


PDApp *PacketDrill::pdapp;

PacketDrill::PacketDrill(PDApp *mod)
{
    pdapp = mod;
}

PacketDrill::~PacketDrill()
{

}

IPv4Datagram* PacketDrill::makeIPPacket(int protocol, enum direction_t direction, L3Address localAddr, L3Address remoteAddr)
{
    IPv4Datagram *datagram = new IPv4Datagram("IPInject");
    datagram->setVersion(4);
    datagram->setHeaderLength(20);

    if (direction == DIRECTION_INBOUND) {
        datagram->setSrcAddress(remoteAddr.toIPv4());
        datagram->setDestAddress(localAddr.toIPv4());
        datagram->setIdentification(pdapp->getIdInbound());
        pdapp->increaseIdInbound();
    } else if (direction == DIRECTION_OUTBOUND) {
        datagram->setSrcAddress(localAddr.toIPv4());
        datagram->setDestAddress(remoteAddr.toIPv4());
        datagram->setIdentification(pdapp->getIdOutbound());
        pdapp->increaseIdOutbound();
    }
    datagram->setTransportProtocol(protocol);
    datagram->setTimeToLive(31);
    datagram->setMoreFragments(0);
    datagram->setDontFragment(0);
    datagram->setFragmentOffset(0);
    datagram->setTypeOfService(0);
    datagram->setByteLength(20);
    return datagram;
}


cPacket* PacketDrill::buildUDPPacket(int address_family, enum direction_t direction,
                                     uint16 udp_payload_bytes, char **error)
{
    PDApp *app = PacketDrill::pdapp;
    UDPPacket *udpPacket = new UDPPacket("UDPInject");
    udpPacket->setByteLength(8);
    cPacket *payload = new cPacket("payload");
    payload->setByteLength(udp_payload_bytes);
    udpPacket->encapsulate(payload);
    IPv4Datagram *ipDatagram = PacketDrill::makeIPPacket(IP_PROT_UDP, direction, app->getLocalAddress(), app->getRemoteAddress());
    if (direction == DIRECTION_INBOUND) {
        udpPacket->setSourcePort(app->getRemotePort());
        udpPacket->setDestinationPort(app->getLocalPort());
        udpPacket->setName("UDPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        udpPacket->setSourcePort(app->getLocalPort());
        udpPacket->setDestinationPort(app->getRemotePort());
        udpPacket->setName("UDPOutbound");
    }

    ipDatagram->encapsulate(udpPacket);
    cPacket* pkt = ipDatagram->dup();
    delete ipDatagram;
    return pkt;
}

int PacketDrill::evaluate(PDExpression *in, PDExpression *out, char **error)
{
    int result = STATUS_OK;
    int64 number;
    out->setType(in->getType());

    if ((in->getType() <= EXPR_NONE) || (in->getType() >= NUM_EXPR_TYPES)) {
        EV_ERROR << "bad expression type: " << in->getType() << endl;
        return STATUS_ERR;
    }
    switch (in->getType()) {
    case EXPR_INTEGER:
        out->setNum(in->getNum());
        break;

    case EXPR_WORD:
        out->setType(EXPR_INTEGER);
        if (in->symbolToInt(in->getString(), &number, error))
            return STATUS_ERR;
        out->setNum(number);
        break;

    case EXPR_STRING:
        if (out->unescapeCstringExpression(in->getString(), error))
            return STATUS_ERR;
        break;

    case EXPR_BINARY:
        result = evaluate_binary_expression(in, out, error);
        break;

    case EXPR_LIST:
        result = evaluateListExpression(in, out, error);
        break;

    default:
        EV_INFO << "type " << in->getType() << " not implemented\n";
    }
    return result;
}

/* Return a copy of the given expression list with each expression
 * evaluated (e.g. symbols resolved to ints). On failure, return NULL
 * and fill in *error.
 */
int PacketDrill::evaluateExpressionList(cQueue *in_list, cQueue *out_list, char **error)
{
    cQueue *node_ptr = out_list;
    for (cQueue::Iterator it(*in_list); !it.end(); it++) {
        PDExpression *outExpr = new PDExpression(((PDExpression *)it())->getType());
        if (evaluate((PDExpression *)it(), outExpr, error)) {
            delete(outExpr);
            return STATUS_ERR;
        }
        node_ptr->insert(outExpr);
    }
    return STATUS_OK;
}

int PacketDrill::evaluate_binary_expression(PDExpression *in, PDExpression *out, char **error)
{
    int result = STATUS_ERR;
    assert(in->getType() == EXPR_BINARY);
    assert(in->getBinary());
    out->setType(EXPR_INTEGER);

    PDExpression *lhs = nullptr;
    PDExpression *rhs = nullptr;
    if ((evaluate(in->getBinary()->lhs, lhs, error)) ||
        (evaluate(in->getBinary()->rhs, rhs, error))) {
        delete(rhs);
        delete(lhs);
        return result;
    }
    if (strcmp("|", in->getBinary()->op) == 0) {
        if (lhs->getType() != EXPR_INTEGER) {
            EV_ERROR << "left hand side of | not an integer\n";
        } else if (rhs->getType() != EXPR_INTEGER) {
            EV_ERROR << "right hand side of | not an integer\n";
        } else {
            out->setNum(lhs->getNum() | rhs->getNum());
            result = STATUS_OK;
        }
    } else {
        EV_ERROR << "bad binary operator '" << in->getBinary()->op << "'\n";
    }
    delete(rhs);
    delete(lhs);
    return result;
}

int PacketDrill::evaluateListExpression(PDExpression *in, PDExpression *out, char **error)
{
    assert(in->getType() == EXPR_LIST);
    assert(out->getType() == EXPR_LIST);

    out->setList(new cQueue("listExpression"));
    return evaluateExpressionList(in->getList(), out->getList(), error);
}

