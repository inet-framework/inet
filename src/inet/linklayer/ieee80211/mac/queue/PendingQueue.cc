//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {
namespace ieee80211 {

static int compareMgmtOverData(Packet *a, Packet *b)
{
    int aPri = dynamicPtrCast<const Ieee80211MgmtHeader>(a->peekAtFront<Ieee80211MacHeader>()) ? 1 : 0; // TODO there should really exist a high-performance isMgmtFrame() function!
    int bPri = dynamicPtrCast<const Ieee80211MgmtHeader>(b->peekAtFront<Ieee80211MacHeader>()) ? 1 : 0;
    return bPri - aPri;
}

Register_Packet_Comparator_Function(MgmtOverDataComparator, compareMgmtOverData);

static int compareMgmtOverMulticastOverUnicast(Packet *a, Packet *b)
{
    const auto& aHeader = a->peekAtFront<Ieee80211MacHeader>();
    const auto& bHeader = b->peekAtFront<Ieee80211MacHeader>();
    int aPri = dynamicPtrCast<const Ieee80211MgmtHeader>(aHeader) ? 2 : aHeader->getReceiverAddress().isMulticast() ? 1 : 0;
    int bPri = dynamicPtrCast<const Ieee80211MgmtHeader>(bHeader) ? 2 : bHeader->getReceiverAddress().isMulticast() ? 1 : 0;
    return bPri - aPri;
}

Register_Packet_Comparator_Function(MgmtOverMulticastOverUnicastComparator, compareMgmtOverMulticastOverUnicast);

static int classifyMgmtOverData(Packet *packet)
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        return 0;
    else
        return 1;
}

Register_Packet_Classifier_Function(MgmtOverDataClassifier, classifyMgmtOverData);

static int classifyMgmtOverMulticastOverUnicast(Packet *packet)
{
    const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header))
        return 0;
    else if (header->getReceiverAddress().isMulticast())
        return 1;
    else
        return 2;
}

Register_Packet_Classifier_Function(MgmtOverMulticastOverUnicastClassifier, classifyMgmtOverMulticastOverUnicast);

} /* namespace inet */
} /* namespace ieee80211 */

