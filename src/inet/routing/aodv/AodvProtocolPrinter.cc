//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvProtocolPrinter.h"

namespace inet {
namespace aodv {

Register_Protocol_Printer(&Protocol::aodv, AodvProtocolPrinter);

void AodvProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto controlPacket = dynamicPtrCast<const AodvControlPacket>(chunk)) {
        context.infoColumn << "(AODV) ";
        switch (controlPacket->getPacketType()) {
            case RREQ: {
                const auto& rreq = CHK(dynamicPtrCast<const Rreq>(chunk));
                context.infoColumn << "Type: 1 (RREQ) (chunkLength = " << rreq->getChunkLength() << "), "
                        << "joinFlag: " << rreq->getJoinFlag() << ", "
                        << "repairFlag: " << rreq->getRepairFlag() << ", "
                        << "gratuitousFlag: " << rreq->getGratuitousRREPFlag() << ", "
                        << "destOnlyFlag: " << rreq->getDestOnlyFlag() << ", "
                        << "unknownSeqNumFlag: " << rreq->getUnknownSeqNumFlag() << ", "
                        << "hopCount: " << rreq->getHopCount() << ", "
                        << "rreqId: " << rreq->getRreqId() << ", "
                        << "destAddr: " << rreq->getDestAddr().toIpv4() << ", "
                        << "destSeqNum: " << rreq->getDestSeqNum() << ", "
                        << "originatorAddr: " << rreq->getOriginatorAddr().toIpv4() << ", "
                        << "originatorSeqNum: " << rreq->getOriginatorSeqNum();
                break;
            }
            case RREQ_IPv6: {
                const auto& rreq = CHK(dynamicPtrCast<const Rreq>(chunk));
                context.infoColumn << "Type: 16 (RREQ_IPv6) (chunkLength = " << rreq->getChunkLength() << "), "
                        << "joinFlag: " << rreq->getJoinFlag() << ", "
                        << "repairFlag: " << rreq->getRepairFlag() << ", "
                        << "gratuitousFlag: " << rreq->getGratuitousRREPFlag() << ", "
                        << "destOnlyFlag: " << rreq->getDestOnlyFlag() << ", "
                        << "unknownSeqNumFlag: " << rreq->getUnknownSeqNumFlag() << ", "
                        << "hopCount: " << rreq->getHopCount() << ", "
                        << "rreqId: " << rreq->getRreqId() << ", "
                        << "destIpAddr: " << rreq->getDestAddr().toIpv6() << ", "
                        << "destSeqNum: " << rreq->getDestSeqNum() << ", "
                        << "originatorIpAddr: " << rreq->getOriginatorAddr().toIpv6() << ", "
                        << "originatorSeqNum: " << rreq->getOriginatorSeqNum();
                break;
            }
            case RREP: {
                const auto& rrep = CHK(dynamicPtrCast<const Rrep>(chunk));
                context.infoColumn << "Type: 2 (RREP) (chunkLength = " << rrep->getChunkLength() << "), "
                        << "repairFlag: " << rrep->getRepairFlag() << ", "
                        << "ackRequiredFlag: " << rrep->getAckRequiredFlag() << ", "
                        << "prefixSize: " << rrep->getPrefixSize() << ", "
                        << "hopCount: " << rrep->getHopCount() << ", "
                        << "destAddr: " << rrep->getDestAddr().toIpv4() << ", "
                        << "destSeqNum: " << rrep->getDestSeqNum() << ", "
                        << "originatorAddr: " << rrep->getOriginatorAddr().toIpv4() << ", "
                        << "lifeTime: " << rrep->getLifeTime().inUnit(SIMTIME_MS);
                break;
            }
            case RREP_IPv6: {
                const auto& rrep = CHK(dynamicPtrCast<const Rrep>(chunk));
                context.infoColumn << "Type: 17 (RREP_IPv6) (chunkLength = " << rrep->getChunkLength() << "), "
                        << "repairFlag: " << rrep->getRepairFlag() << ", "
                        << "ackRequiredFlag: " << rrep->getAckRequiredFlag() << ", "
                        << "prefixSize: " << rrep->getPrefixSize() << ", "
                        << "hopCount: " << rrep->getHopCount() << ", "
                        << "destAddr: " << rrep->getDestAddr().toIpv6() << ", "
                        << "destSeqNum: " << rrep->getDestSeqNum() << ", "
                        << "originatorAddr: " << rrep->getOriginatorAddr().toIpv6() << ", "
                        << "lifeTime: " << rrep->getLifeTime().inUnit(SIMTIME_MS);
                break;
            }
            case RERR: {
                const auto& rerr = CHK(dynamicPtrCast<const Rerr>(chunk));
                context.infoColumn << "Type: 3 (RERR) (chunkLength = " << rerr->getChunkLength() << "), "
                        << "destCount: " << rerr->getUnreachableNodesArraySize();
                for (uint8_t index = 0; index < rerr->getUnreachableNodesArraySize(); ++index) {
                    context.infoColumn  << ", " << "unreachableDestAddr (" << index + 1 << ") :"
                            << rerr->getUnreachableNodes(rerr->getUnreachableNodesArraySize() - (index + 1)).addr.toIpv4() << ", "
                            << "unreachableDestSeqNum (" << index + 1 << ") :"
                            << rerr->getUnreachableNodes(rerr->getUnreachableNodesArraySize() - (index + 1)).seqNum;
                }
                break;
            }
            case RERR_IPv6: {
                const auto& rerr = CHK(dynamicPtrCast<const Rerr>(chunk));
                context.infoColumn << "Type: 18 (RERR_IPv6) (chunkLength = " << rerr->getChunkLength() << "), "
                        << "destCount: " << rerr->getUnreachableNodesArraySize();
                for (uint8_t index = 0; index < rerr->getUnreachableNodesArraySize(); ++index) {
                    context.infoColumn  << ", " << "unreachableDestAddr (" << index + 1 << ") :"
                            << rerr->getUnreachableNodes(rerr->getUnreachableNodesArraySize() - (index + 1)).addr.toIpv6() << ", "
                            << "unreachableDestSeqNum (" << index + 1 << ") :"
                            << rerr->getUnreachableNodes(rerr->getUnreachableNodesArraySize() - (index + 1)).seqNum;
                }
                break;
            }
            case RREPACK: {
                const auto& rrepAck = CHK(dynamicPtrCast<const RrepAck>(chunk));
                context.infoColumn << "Type: 4 (RREPACK) (chunkLength = " << rrepAck->getChunkLength() << ")";
                break;
            }
            case RREPACK_IPv6: {
                const auto& rrepAck = CHK(dynamicPtrCast<const RrepAck>(chunk));
                context.infoColumn << "Type: 19 (RREPACK_IPv6) (chunkLength = " << rrepAck->getChunkLength() << ")";
                break;
            }
            default: {
                context.infoColumn << "(AODV: unknown type) " << chunk;
            }
        }
    }
    else
        context.infoColumn << "(AODV) " << chunk;
}

} // namespace aodv
} // namespace inet

