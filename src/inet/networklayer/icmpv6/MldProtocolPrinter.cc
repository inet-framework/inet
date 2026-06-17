//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::mld, MldProtocolPrinter);

void MldProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    // MLDv2 messages first (Mldv2Query is itself an MldMessage with type 130, so it must
    // be matched before the generic MLDv1 branch below).
    if (auto report = dynamicPtrCast<const Mldv2Report>(chunk)) {
        context.infoColumn << "MLDv2 REPORT, " << report->getMulticastAddressRecordArraySize() << " record(s)";
        return;
    }
    if (auto q2 = dynamicPtrCast<const Mldv2Query>(chunk)) {
        context.infoColumn << "MLDv2 QRY";
        if (q2->getMulticastAddress().isUnspecified())
            context.infoColumn << ", general";
        else
            context.infoColumn << ", group " << q2->getMulticastAddress();
        context.infoColumn << ", " << q2->getSourceList().size() << " source(s)";
        return;
    }
    if (auto header = dynamicPtrCast<const MldMessage>(chunk)) {
        switch (header->getType()) {
            case ICMPv6_MLD_QUERY: {
                auto q = dynamicPtrCast<const MldQuery>(chunk);
                context.infoColumn << "MLDv1 QRY";
                if (q && q->getMulticastAddress().isUnspecified())
                    context.infoColumn << ", general";
                else if (q)
                    context.infoColumn << ", group " << q->getMulticastAddress();
                if (q)
                    context.infoColumn << ", maxRespDelay=" << q->getMaxRespDelay() << "ms";
                break;
            }
            case ICMPv6_MLD_REPORT: {
                auto r = dynamicPtrCast<const MldReport>(chunk);
                context.infoColumn << "MLDv1 REPORT";
                if (r)
                    context.infoColumn << ", group " << r->getMulticastAddress();
                break;
            }
            case ICMPv6_MLD_DONE: {
                auto d = dynamicPtrCast<const MldDone>(chunk);
                context.infoColumn << "MLDv1 DONE";
                if (d)
                    context.infoColumn << ", group " << d->getMulticastAddress();
                break;
            }
            default:
                context.infoColumn << "MLD type=" << header->getType();
                break;
        }
    }
    else
        context.infoColumn << "(MLD) " << chunk;
}

} // namespace inet
