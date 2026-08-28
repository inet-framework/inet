//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

Register_Protocol_Printer(&Protocol::ieee80211Mgmt, Ieee80211MgmtProtocolPrinter);

void Ieee80211MgmtProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    context.infoColumn << "(IEEE 802.11 Mgmt) " << chunk;
    auto frame = dynamicPtrCast<const Ieee80211MgmtFrame>(chunk);
    if (frame != nullptr && frame->getHtCapabilitiesPresent()) {
        const auto& capabilities = frame->getHtCapabilities();
        int maximumRxMcs = -1;
        for (int mcs = 0; mcs < 77; mcs++)
            if (capabilities.rxMcsSupported[mcs])
                maximumRxMcs = mcs;
        context.infoColumn << " HT-Cap(width=" << (capabilities.supportedChannelWidth40Mhz ? "20/40" : "20")
                << "MHz,rxMcs=0.." << maximumRxMcs << ",tx="
                << (capabilities.txMcsSetDefined ? "defined" : "undefined") << ")";
    }
    if (frame != nullptr && frame->getHtOperationPresent()) {
        const auto& operation = frame->getHtOperation();
        int maximumBasicMcs = -1;
        for (int mcs = 0; mcs < 77; mcs++)
            if (operation.basicMcsSupported[mcs])
                maximumBasicMcs = mcs;
        context.infoColumn << " HT-Op(primary=" << operation.primaryChannel << ",width="
                << (operation.staChannelWidth40Mhz ? 40 : 20) << "MHz,basicMcs=0.." << maximumBasicMcs << ")";
    }
}

} // namespace ieee80211
} // namespace inet
