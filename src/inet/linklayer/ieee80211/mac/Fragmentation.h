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

#ifndef __INET_FRAGMENTATION_H
#define __INET_FRAGMENTATION_H

#include "IFragmentation.h"

namespace inet {
namespace ieee80211 {

class INET_API FragmentationNotSupported : public IFragmenter, public cObject
{
    public:
        virtual std::vector<Ieee80211DataOrMgmtFrame*> fragment(Ieee80211DataOrMgmtFrame *frame, int fragmentationThreshold);
};

class INET_API ReassemblyNotSupported : public IReassembly, public cObject
{
    public:
        virtual Ieee80211DataOrMgmtFrame *addFragment(Ieee80211DataOrMgmtFrame *frame);
        virtual void purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber);
};

class INET_API BasicFragmentation : public IFragmenter, public cObject
{
    public:
        virtual std::vector<Ieee80211DataOrMgmtFrame*> fragment(Ieee80211DataOrMgmtFrame *frame, int fragmentationThreshold);
};

class INET_API BasicReassembly : public IReassembly, public cObject
{
    protected:
        struct Key {
            MACAddress macAddress;
            uint8_t tid;
            uint16_t seqNum;
            bool operator == (const Key& o) const { return macAddress == o.macAddress && tid == o.tid && seqNum == o.seqNum; }
            bool operator < (const Key& o) const { return macAddress < o.macAddress || (macAddress == o.macAddress && (tid < o.tid || (tid == o.tid && seqNum < o.seqNum))); }
        };
        struct Value {
            Ieee80211DataOrMgmtFrame *frame = nullptr;
            uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
            uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
        };
        typedef std::map<Key,Value> FragmentsMap;
        FragmentsMap fragmentsMap;
    public:
        virtual ~BasicReassembly();
        virtual Ieee80211DataOrMgmtFrame *addFragment(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber) override;
};

} // namespace ieee80211
} // namespace inet

#endif
