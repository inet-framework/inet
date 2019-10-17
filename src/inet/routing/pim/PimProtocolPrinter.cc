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
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/PimProtocolPrinter.h"

namespace inet {

Register_Protocol_Printer(&Protocol::pim, PimProtocolPrinter);

namespace {

std::string getEncodedUnicastAddress(const EncodedUnicastAddress& addr)
{
    std::stringstream s;
    s << "unicastAddr: " << addr.unicastAddress.toIpv4();
    return s.str();
}

std::string getEncodedGroupAddress(const EncodedGroupAddress& addr)
{
    std::stringstream s;
    s << "bBit: " << addr.B << ", "
            << "zBit: " << addr.Z << ", "
            << "maskLength: " << addr.maskLength << ", "
            << "groupAddr: " << addr.groupAddress.toIpv4();
    return s.str();
}

std::string getEncodedSourceAddress(const EncodedSourceAddress& addr)
{
    std::stringstream s;
    s << "sBit: " << addr.S << ", "
            << "wBit: " << addr.W << ", "
            << "rBit: " << addr.R << ", "
            << "maskLength: " << addr.maskLength << ", "
            << "sourceAddr: " << addr.sourceAddress.toIpv4();
    return s.str();
}

} // namespace

void PimProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto pimPacket = dynamicPtrCast<const PimPacket>(chunk)) {
        context.infoColumn << "(PIM) (chunkLength = " << pimPacket->getChunkLength() << "), "
                << "version: " << pimPacket->getVersion() << ", "
                << "type: " << pimPacket->getType() << ", "
                << "crc: " << pimPacket->getCrc();
        switch (pimPacket->getType()) {
            case Hello: {
                const auto& pimHello = staticPtrCast<const PimHello>(chunk);
                for (size_t index = 0; index < pimHello->getOptionsArraySize(); ++index) {
                    switch (pimHello->getOptions(index)->getType()) {
                        case Holdtime: {
                            const HoldtimeOption* holdtimeOption = static_cast<const HoldtimeOption*>(pimHello->getOptions(index));
                            context.infoColumn << ", " << "optionType: 1" << ", "
                                    << "optionLength: 2" << ", "
                                    << "optionValue: holdtime: " << holdtimeOption->getHoldTime();
                            break;
                        }
                        case LANPruneDelay: {
                            const LanPruneDelayOption* lanPruneDelayOption = static_cast<const LanPruneDelayOption*>(pimHello->getOptions(index));
                            context.infoColumn << ", " << "optionType: 2" << ", "
                                    << "optionLength: 4" << ", "
                                    << "optionValue: tBit: 0" << ", "    // FIXME: T bit missing
                                    << "propagationDelay: " << lanPruneDelayOption->getPropagationDelay() << ", "
                                    << "overrideInterval: " << lanPruneDelayOption->getOverrideInterval();
                            break;
                        }
                        case DRPriority: {
                            const DrPriorityOption* drPriorityOption = static_cast<const DrPriorityOption*>(pimHello->getOptions(index));
                            context.infoColumn << ", " << "optionType: 19" << ", "
                                    << "optionLength: 4" << ", "
                                    << "optionValue: drPriority: " << drPriorityOption->getPriority();
                            break;
                        }
                        case GenerationID: {
                            const GenerationIdOption* generationIdOption = static_cast<const GenerationIdOption*>(pimHello->getOptions(index));
                            context.infoColumn << ", " << "optionType: 20" << ", "
                                    << "optionLength: 4" << ", "
                                    << "optionValue: generationId: " << generationIdOption->getGenerationID();
                            break;
                        }
                        default:
                            context.infoColumn << "Unknown Hello option type!";
                    }
                }
                break;
            }
            case Register: {
                const auto& pimRegister = staticPtrCast<const PimRegister>(chunk);
                context.infoColumn << ", " << "bBit: " << pimRegister->getB() << ", "
                        << "nBit: " << pimRegister->getN();
                break;
            }
            case RegisterStop: {
                const auto& pimRegisterStop = staticPtrCast<const PimRegisterStop>(chunk);
                context.infoColumn << ", " << "group: " << getEncodedGroupAddress(pimRegisterStop->getGroupAddress()) << ", ";
                context.infoColumn << "source: " << getEncodedUnicastAddress(pimRegisterStop->getSourceAddress());
                break;
            }
            case Graft:
            case GraftAck:
            case JoinPrune: {
                const auto& pimJoinPrune = staticPtrCast<const PimJoinPrune>(chunk);
                context.infoColumn << ", " << "upstreamNeighbor: " << getEncodedUnicastAddress(pimJoinPrune->getUpstreamNeighborAddress()) << ", ";
                context.infoColumn << "numGroups: " << pimJoinPrune->getJoinPruneGroupsArraySize() << ", "
                        << "holdtime: " << pimJoinPrune->getHoldTime();
                for (size_t i = 0; i < pimJoinPrune->getJoinPruneGroupsArraySize(); ++i) {
                    auto& joinPruneGroups = pimJoinPrune->getJoinPruneGroups(i);
                    context.infoColumn << ", group: " << getEncodedGroupAddress(joinPruneGroups.getGroupAddress()) << ", ";
                    context.infoColumn << "numJoins: " << joinPruneGroups.getJoinedSourceAddressArraySize() << ", "
                            << "numPrunes: " << joinPruneGroups.getPrunedSourceAddressArraySize();
                    for (size_t k = 0; k < joinPruneGroups.getJoinedSourceAddressArraySize(); ++k) {
                        context.infoColumn << ", joinedSource: " << getEncodedSourceAddress(joinPruneGroups.getJoinedSourceAddress(k));
                    }
                    for (size_t k = 0; k < joinPruneGroups.getPrunedSourceAddressArraySize(); ++k) {
                        context.infoColumn << ", prunedSource: " << getEncodedSourceAddress(joinPruneGroups.getPrunedSourceAddress(k));
                    }
                }
                break;
            }
            case Assert: {
                const auto& pimAssert = staticPtrCast<const PimAssert>(chunk);
                context.infoColumn << ", " << "type: 5" << ", ";
                context.infoColumn << "group: "<< getEncodedGroupAddress(pimAssert->getGroupAddress()) << ", ";
                context.infoColumn << "source: "<< getEncodedUnicastAddress(pimAssert->getSourceAddress()) << ", ";
                context.infoColumn << "rBit: " << pimAssert->getR() << ", "
                        << "metricPreference: " << pimAssert->getMetricPreference() << ", "
                        << "metric: " << pimAssert->getMetric();
                break;
            }
            case StateRefresh: {
                const auto& pimStateRefresh = staticPtrCast<const PimStateRefresh>(chunk);
                context.infoColumn << "type: 9" << ", ";
                context.infoColumn << "group: " << getEncodedGroupAddress(pimStateRefresh->getGroupAddress()) << ", ";
                context.infoColumn << "source: " << getEncodedUnicastAddress(pimStateRefresh->getSourceAddress()) << ", ";
                context.infoColumn << "originator: " << getEncodedUnicastAddress(pimStateRefresh->getOriginatorAddress()) << ", ";
                context.infoColumn << "rBit: " << pimStateRefresh->getR() << ", "
                        << "metricPreference: " << pimStateRefresh->getMetricPreference() << ", "
                        << "metric: " << pimStateRefresh->getMetric() << ", "
                        << "masklen: " << pimStateRefresh->getMaskLen() << ", "
                        << "ttl: " << pimStateRefresh->getTtl() << ", "
                        << "pBit: " << pimStateRefresh->getP() << ", "
                        << "nBit: " << pimStateRefresh->getN() << ", "
                        << "oBit: " << pimStateRefresh->getO() << ", "
                        << "interval: " << pimStateRefresh->getInterval();
                break;
            }
            default:
                context.infoColumn << "(PIM: unknown type) " << chunk;
        }

    }
    else
        context.infoColumn << "(PIM) " << chunk;
}

} // namespace inet

