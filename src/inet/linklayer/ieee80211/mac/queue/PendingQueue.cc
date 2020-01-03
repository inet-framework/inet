//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {
namespace ieee80211 {

static int compareMgmtOverData(Packet *a, Packet *b)
{
    int aPri = dynamicPtrCast<const Ieee80211MgmtHeader>(a->peekAtFront<Ieee80211MacHeader>()) ? 1 : 0;  //TODO there should really exist a high-performance isMgmtFrame() function!
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
