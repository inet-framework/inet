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

#ifndef __INET_BASICREASSEMBLY_H
#define __INET_BASICREASSEMBLY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"

namespace inet {
namespace ieee80211 {

class INET_API BasicReassembly : public IReassembly, public cObject
{
    protected:
        struct Key {
            MacAddress macAddress;
            Tid tid;
            SequenceNumber seqNum;
            bool operator == (const Key& o) const { return macAddress == o.macAddress && tid == o.tid && seqNum == o.seqNum; }
            bool operator < (const Key& o) const { return macAddress < o.macAddress || (macAddress == o.macAddress && (tid < o.tid || (tid == o.tid && seqNum < o.seqNum))); }
        };
        struct Value {
            std::vector<Packet *> fragments;
            uint16_t receivedFragments = 0; // each bit corresponds to a fragment number
            uint16_t allFragments = 0; // bits for all fragments set to one (0..numFragments-1); 0 means unfilled
        };
        typedef std::map<Key,Value> FragmentsMap;
        FragmentsMap fragmentsMap;
    public:
        virtual ~BasicReassembly();
        virtual Packet *addFragment(Packet *packet) override;
        virtual void purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber) override;
};

} // namespace ieee80211
} // namespace inet

#endif

