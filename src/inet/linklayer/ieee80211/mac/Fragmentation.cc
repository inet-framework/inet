//
// Copyright (C) 2015 Opensim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Zoltan Bojthe
//

#include "Fragmentation.h"

#include "Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Register_Class(FragmentationNotSupported);

std::vector<Ieee80211DataOrMgmtFrame*> FragmentationNotSupported::fragment(Ieee80211DataOrMgmtFrame *frame, int fragmentationThreshold)
{
    throw cRuntimeError("Fragmentation not supported");
}

Register_Class(ReassemblyNotSupported);

Ieee80211DataOrMgmtFrame *ReassemblyNotSupported::addFragment(Ieee80211DataOrMgmtFrame *frame)
{
    throw cRuntimeError("Fragmentation reassembly not supported");
}

void ReassemblyNotSupported::purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
}

//---

Register_Class(BasicFragmentation);

std::vector<Ieee80211DataOrMgmtFrame*> BasicFragmentation::fragment(Ieee80211DataOrMgmtFrame *frame, int fragmentationThreshold)
{
    // Notes:
    // 1. only the MSDU is carried in the fragments (i.e. only frame's payload, without the 802.11 header)
    // 2. fragmentationThreshold refers to the MPDU size, i.e. 802.11 header included
    // 3. for convenience, this implementation sends the original frame encapsulated in the last fragment, all other fragments are dummies with no data
    //
    std::vector<Ieee80211DataOrMgmtFrame*> fragments;

    cPacket *payload = frame->decapsulate();
    int payloadLength = payload->getByteLength();
    Ieee80211DataOrMgmtFrame *fragmentHeader = frame->dup();
    int headerLength = fragmentHeader->getByteLength();
    frame->encapsulate(payload); // restore original state

    int maxFragmentPayload = fragmentationThreshold - headerLength;
    if (payloadLength >= maxFragmentPayload * MAX_NUM_FRAGMENTS)
        throw cRuntimeError("Fragmentation: frame \"%s\" too large, won't fit into %d fragments", frame->getName(), MAX_NUM_FRAGMENTS);

    int i = 0;
    for( ; headerLength + payloadLength > fragmentationThreshold; i++) {
        Ieee80211DataOrMgmtFrame *fragment = fragmentHeader->dup();
        fragment->setByteLength(fragmentationThreshold);
        fragment->setFragmentNumber(i);
        fragment->setMoreFragments(true);
        fragments.push_back(fragment);
        payloadLength -= maxFragmentPayload;
    }

    Ieee80211DataOrMgmtFrame *lastFragment = fragmentHeader;
    lastFragment->encapsulate(frame);
    lastFragment->setByteLength(headerLength + payloadLength);
    lastFragment->setFragmentNumber(i);
    lastFragment->setMoreFragments(false);
    fragments.push_back(lastFragment);
    return fragments;
}

//---

Register_Class(BasicReassembly);

Ieee80211DataOrMgmtFrame *BasicReassembly::addFragment(Ieee80211DataOrMgmtFrame *frame)
{
    // find entry for this frame
    Key key;
    key.macAddress = frame->getTransmitterAddress();
    key.tid = -1;
    if (frame->getType() == ST_DATA_WITH_QOS)
        if (Ieee80211DataFrame *qosDataFrame = dynamic_cast<Ieee80211DataFrame *>(frame))
            key.tid = qosDataFrame->getTid();
    key.seqNum = frame->getSequenceNumber();
    short fragNum = frame->getFragmentNumber();
    ASSERT(fragNum >= 0 && fragNum < MAX_NUM_FRAGMENTS);
    auto& value = fragmentsMap[key];

    // update entry
    uint16_t fragmentBit = 1 << fragNum;
    value.receivedFragments |= fragmentBit;
    if (!frame->getMoreFragments())
        value.allFragments = (fragmentBit << 1) - 1;
    if (!value.frame) {
        frame->setByteLength(0);  // needed for decapsulation of larger packet
        value.frame = check_and_cast_nullable<Ieee80211DataOrMgmtFrame *>(frame->decapsulate());
    }
    delete frame;

    // if all fragments arrived, return assembled frame
    if (value.allFragments != 0 && value.allFragments == value.receivedFragments) {
        Ieee80211DataOrMgmtFrame *result = value.frame;
        fragmentsMap.erase(key);
        return result;
    }
    else
        return nullptr;
}

void BasicReassembly::purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
    Key key;
    key.macAddress = address;
    key.tid = tid;
    key.seqNum = startSeqNumber;
    auto itStart = fragmentsMap.lower_bound(key);
    key.seqNum = endSeqNumber;
    auto itEnd = fragmentsMap.upper_bound(key);

    if (endSeqNumber < startSeqNumber) {
        for (auto it = itStart; it != fragmentsMap.end(); ) {
            delete it->second.frame;
            it = fragmentsMap.erase(it);
        }
        itStart = fragmentsMap.begin();
    }
    for (auto it = itStart; it != itEnd; ) {
        delete it->second.frame;
        it = fragmentsMap.erase(it);
    }
}

BasicReassembly::~BasicReassembly()
{
    for (auto it = fragmentsMap.begin(); it != fragmentsMap.end(); ++it)
        delete it->second.frame;
}

} // namespace ieee80211
} // namespace inet
