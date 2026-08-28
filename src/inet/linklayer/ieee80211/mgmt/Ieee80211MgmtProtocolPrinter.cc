//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <sstream>

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

Register_Protocol_Printer(&Protocol::ieee80211Mgmt, Ieee80211MgmtProtocolPrinter);

static std::string formatMcsBitmap(const bool (&mcsSupported)[77])
{
    std::ostringstream stream;
    stream << "{";
    bool first = true;
    for (int mcs = 0; mcs < 77; ) {
        if (!mcsSupported[mcs]) {
            mcs++;
            continue;
        }
        int firstMcs = mcs;
        while (mcs + 1 < 77 && mcsSupported[mcs + 1])
            mcs++;
        if (!first)
            stream << ",";
        stream << firstMcs;
        if (firstMcs != mcs)
            stream << ".." << mcs;
        first = false;
        mcs++;
    }
    stream << "}";
    return stream.str();
}

void Ieee80211MgmtProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    context.infoColumn << "(IEEE 802.11 Mgmt) " << chunk;
    auto frame = dynamicPtrCast<const Ieee80211MgmtFrame>(chunk);
    if (frame != nullptr && frame->getHtCapabilitiesPresent()) {
        const auto& capabilities = frame->getHtCapabilities();
        context.infoColumn << " HT-Cap(width=" << (capabilities.supportedChannelWidth40Mhz ? "20/40" : "20")
                << "MHz,rxMcs=" << formatMcsBitmap(capabilities.rxMcsSupported) << ",tx="
                << (capabilities.txMcsSetDefined ? "defined" : "undefined") << ")";
    }
    if (frame != nullptr && frame->getHtOperationPresent()) {
        const auto& operation = frame->getHtOperation();
        context.infoColumn << " HT-Op(primary=" << operation.primaryChannel << ",width="
                << (operation.staChannelWidth40Mhz ? 40 : 20) << "MHz,basicMcs=" << formatMcsBitmap(operation.basicMcsSupported) << ")";
    }
}

} // namespace ieee80211
} // namespace inet
