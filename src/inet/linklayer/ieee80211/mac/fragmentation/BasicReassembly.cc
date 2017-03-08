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

#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Register_Class(BasicReassembly);

/*
 * FIXME: this function needs a serious review
 */
Ieee80211DataOrMgmtFrame *BasicReassembly::addFragment(Ieee80211DataOrMgmtFrame *frame)
{
    // Frame is not fragmented
    if (!frame->getMoreFragments() && frame->getFragmentNumber() == 0)
        return frame;
    // FIXME: temporary fix for mgmt frames
    if (dynamic_cast<Ieee80211ManagementFrame*>(frame))
        return frame;
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
    MACAddress txAddress = frame->getTransmitterAddress();
    delete frame;

    // if all fragments arrived, return assembled frame
    if (value.allFragments != 0 && value.allFragments == value.receivedFragments) {
        Ieee80211DataOrMgmtFrame *result = value.frame;
        // We need to restore some data from the carrying frame's header like TX address
        // TODO: Maybe we need to restore the fromDs, toDs fields as well when traveling through multiple APs
        // TODO: Are there any other fields that we need to restore?
        // TODO: SET TX ADDR WHEN THE FRAME ARRIVES FROM UPPER LAYER
        result->setTransmitterAddress(txAddress);
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

