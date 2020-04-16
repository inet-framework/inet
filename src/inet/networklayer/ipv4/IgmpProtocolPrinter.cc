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
// @author: Zoltan Bojthe
//

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"
#include "inet/networklayer/ipv4/IgmpProtocolPrinter.h"
#include "inet/networklayer/ipv4/Igmpv3.h"

namespace inet {

Register_Protocol_Printer(&Protocol::igmp, IgmpProtocolPrinter);

void IgmpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const IgmpMessage>(chunk)) {
        switch (header->getType()) {
            case IGMP_MEMBERSHIP_QUERY:
                if (auto header = dynamicPtrCast<const Igmpv1Query>(chunk)) {
                    context.infoColumn << "IGMPv1 QRY";
                    if (header->getGroupAddress().isUnspecified())
                        context.infoColumn << ", general";
                    else
                        context.infoColumn << ", group " << header->getGroupAddress();
                }
                else if (auto header = dynamicPtrCast<const Igmpv2Query>(chunk)) {
                    context.infoColumn << "IGMPv2 QRY";
                    if (header->getGroupAddress().isUnspecified())
                        context.infoColumn << ", general";
                    else
                        context.infoColumn << ", group " << header->getGroupAddress();
                    context.infoColumn << ", maxRespTime=" << SimTime(header->getMaxRespTimeCode(), (SimTimeUnit)-1);
                }
                else if (auto header = dynamicPtrCast<const Igmpv3Query>(chunk)) {
                    context.infoColumn << "IGMPv3 QRY";
                    if (header->getGroupAddress().isUnspecified())
                        context.infoColumn << ", general";
                    else
                        context.infoColumn << ", group " << header->getGroupAddress();
                    context.infoColumn << ", maxRespTime=" << SimTime(Igmpv3::decodeTime(header->getMaxRespTimeCode()), (SimTimeUnit)-1);
                    if (header->getSuppressRouterProc())
                        context.infoColumn << " Suppress";
                    context.infoColumn << ", QRV=" << header->getRobustnessVariable();
                    context.infoColumn << ", QQIC=" << SimTime(Igmpv3::decodeTime(header->getQueryIntervalCode()), SIMTIME_S);
                    if (header->getSourceList().size() > 0) {
                        context.infoColumn << ", SRC={";
                        for(auto it = header->getSourceList().begin(); it != header->getSourceList().end(); ++it) {
                            if (it != header->getSourceList().begin())
                                context.infoColumn << ", ";
                            context.infoColumn << *it;
                        }
                        context.infoColumn << "}";
                    }
                }
                else {
                    context.infoColumn << "IGMP QRY";
                    if (header->getGroupAddress().isUnspecified())
                        context.infoColumn << ", general";
                    else
                        context.infoColumn << ", group " << header->getGroupAddress();
                }
                break;
            case IGMPV1_MEMBERSHIP_REPORT:
                context.infoColumn << "IGMPv1 REPORT";
                if (auto header = dynamicPtrCast<const Igmpv1Report>(chunk)) {
                }
                break;
            case IGMPV2_MEMBERSHIP_REPORT:
                context.infoColumn << "IGMPv2 REPORT";
                if (auto header = dynamicPtrCast<const Igmpv2Report>(chunk)) {
                }
                break;
            case IGMPV2_LEAVE_GROUP:
                context.infoColumn << "IGMPv2 LEAVE";
                if (auto header = dynamicPtrCast<const Igmpv2Leave>(chunk)) {
                }
                break;
            case IGMPV3_MEMBERSHIP_REPORT:
                context.infoColumn << "IGMPv3 REPORT";
                if (auto header = dynamicPtrCast<const Igmpv3Report>(chunk)) {
                }
                break;
            default:
                context.infoColumn << " type=" << header->getType();
                break;
        }
    }
    else
        context.infoColumn << "(IGMP) " << chunk;
}

} // namespace inet

