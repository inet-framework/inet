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

#include "PacketDrill.h"
#include "PacketDrillUtils.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/transportlayer/tcp_common/TCPSegment_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"

using namespace inet;
using namespace tcp;

PacketDrillApp *PacketDrill::pdapp = nullptr;

PacketDrill::PacketDrill(PacketDrillApp *mod)
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
    } else
        throw cRuntimeError("Unknown direction type %d", direction);
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
    PacketDrillApp *app = PacketDrill::pdapp;
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
    } else
        throw cRuntimeError("Unknown direction");

    ipDatagram->encapsulate(udpPacket);
    cPacket* pkt = ipDatagram->dup();
    delete ipDatagram;
    return pkt;
}


TCPOption *setOptionValues(PacketDrillTcpOption* opt)
{
    unsigned char length = opt->getLength();
    switch (opt->getKind()) {
        case TCPOPT_EOL: // EOL
            return new TCPOptionEnd();
        case TCPOPT_NOP: // NOP
            return new TCPOptionNop();
        case TCPOPT_MAXSEG:
            if (length == 4) {
                auto *option = new TCPOptionMaxSegmentSize();
                option->setLength(length);
                option->setMaxSegmentSize(opt->getMss());
                return option;
            }
            break;
        case TCPOPT_WINDOW:
            if (length == 3) {
                auto *option = new TCPOptionWindowScale();
                option->setLength(length);
                option->setWindowScale(opt->getWindowScale());
                return option;
            }
            break;
        case TCPOPT_SACK_PERMITTED:
            if (length == 2) {
                auto *option = new TCPOptionSackPermitted();
                option->setLength(length);
                return option;
            }
            break;
        case TCPOPT_SACK:
            if (length > 2 && (length % 8) == 2) {
                auto *option = new TCPOptionSack();
                option->setLength(length);
                option->setSackItemArraySize(length / 8);
                unsigned int count = 0;
                for (int i = 0; i < 2 * opt->getBlockList()->getLength(); i += 2) {
                    SackItem si;
                    si.setStart(((PacketDrillStruct *) opt->getBlockList()->get(i))->getValue1());
                    si.setEnd(((PacketDrillStruct *) opt->getBlockList()->get(i))->getValue2());
                    option->setSackItem(count++, si);
                }
                return option;
            }
            break;
        case TCPOPT_TIMESTAMP:
            if (length == 10) {
                auto *option = new TCPOptionTimestamp();
                option->setLength(length);
                option->setSenderTimestamp(opt->getVal());
                option->setEchoedTimestamp(opt->getEcr());
                return option;
            }
            break;
        default:
            EV_INFO << "TCP option is not supported (yet).";
            break;
    } // switch
    auto *option = new TCPOptionUnknown();
    option->setKind(opt->getKind());
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, length);
    return option;
}

cPacket* PacketDrill::buildTCPPacket(int address_family, enum direction_t direction, const char *flags,
                                     uint32 startSequence, uint16 tcpPayloadBytes, uint32 ackSequence,
                                     int32 window, cQueue *tcpOptions, char **error)
{
    PacketDrillApp *app = PacketDrill::pdapp;
    TCPSegment *tcpseg = new TCPSegment("TCPInject");

    // fill TCP header structure
    if (direction == DIRECTION_INBOUND) {
        tcpseg->setSrcPort(app->getRemotePort());
        tcpseg->setDestPort(app->getLocalPort());
        tcpseg->setName("TCPInbound");
    } else if (direction == DIRECTION_OUTBOUND) {
        tcpseg->setSrcPort(app->getLocalPort());
        tcpseg->setDestPort(app->getRemotePort());
        tcpseg->setName("TCPOutbound");
    }
    tcpseg->setSequenceNo(startSequence);
    tcpseg->setAckNo(ackSequence);

    tcpseg->setHeaderLength(TCP_HEADER_OCTETS);

    // set flags
    tcpseg->setFinBit(strchr(flags, 'F'));
    tcpseg->setSynBit(strchr(flags, 'S'));
    tcpseg->setRstBit(strchr(flags, 'R'));
    tcpseg->setPshBit(strchr(flags, 'P'));
    tcpseg->setAckBit(strchr(flags, '.'));
    tcpseg->setUrgBit(0);
    if (tcpseg->getSynBit() && !tcpseg->getAckBit())
        tcpseg->setName("Inject SYN");
    else if (tcpseg->getSynBit() && tcpseg->getAckBit())
        tcpseg->setName("Inject SYN ACK");
    else if (!tcpseg->getSynBit() && tcpseg->getAckBit() && !tcpseg->getPshBit())
        tcpseg->setName("Inject ACK");
    else if (tcpseg->getPshBit())
        tcpseg->setName("Inject PUSH");
    else if (tcpseg->getFinBit())
        tcpseg->setName("Inject FIN");

    tcpseg->setWindow(window);
    tcpseg->setUrgentPointer(0);
    // Checksum (header checksum): modelled by cMessage::hasBitError()

    uint16 tcpOptionsLength = 0;

    if (tcpOptions && tcpOptions->getLength() > 0) { // options present?
        TCPOption *option;
        uint16 optionsCounter = 0;

        for (cQueue::Iterator iter(*tcpOptions); !iter.end(); iter++) {
            PacketDrillTcpOption* opt = (PacketDrillTcpOption*)(iter());

            option = setOptionValues(opt);
            tcpOptionsLength += opt->getLength();
            // write option to tcp header
            tcpseg->addHeaderOption(option);
            optionsCounter++;
        } // for
    } // if options present
    tcpseg->setHeaderLength(tcpseg->getHeaderLength() + tcpOptionsLength);
    tcpseg->setByteLength(tcpseg->getHeaderLength() + tcpPayloadBytes);
    tcpseg->setPayloadLength(tcpPayloadBytes);

    IPv4Datagram *ipDatagram = PacketDrill::makeIPPacket(IP_PROT_TCP, direction, app->getLocalAddress(),
            app->getRemoteAddress());
    ipDatagram->encapsulate(tcpseg);
    cPacket* pkt = ipDatagram->dup();
    delete tcpOptions;
    delete ipDatagram;
    return pkt;
}


int PacketDrill::evaluate(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    int result = STATUS_OK;
    int64 number;
    out->setType(in->getType());

    if ((in->getType() <= EXPR_NONE) || (in->getType() >= NUM_EXPR_TYPES)) {
        EV_ERROR << "bad expression type: " << in->getType() << endl;
        return STATUS_ERR;
    }
    switch (in->getType()) {
    case EXPR_ELLIPSIS:
        break;

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

    case EXPR_NONE:
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
        PacketDrillExpression *outExpr = new PacketDrillExpression(((PacketDrillExpression *)(it()))->getType());
        if (evaluate((PacketDrillExpression *)(it()), outExpr, error)) {
            delete(outExpr);
            return STATUS_ERR;
        }
        node_ptr->insert(outExpr);
    }
    return STATUS_OK;
}

int PacketDrill::evaluate_binary_expression(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    int result = STATUS_ERR;
    assert(in->getType() == EXPR_BINARY);
    assert(in->getBinary());
    out->setType(EXPR_INTEGER);

    PacketDrillExpression *lhs = nullptr;
    PacketDrillExpression *rhs = nullptr;
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

int PacketDrill::evaluateListExpression(PacketDrillExpression *in, PacketDrillExpression *out, char **error)
{
    assert(in->getType() == EXPR_LIST);
    assert(out->getType() == EXPR_LIST);

    out->setList(new cQueue("listExpression"));
    return evaluateExpressionList(in->getList(), out->getList(), error);
}

