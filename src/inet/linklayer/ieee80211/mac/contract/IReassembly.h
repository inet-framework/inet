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

#ifndef __INET_IREASSEMBLY_H
#define __INET_IREASSEMBLY_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtHeader;

/**
 * Abstract interface for classes that encapsulate the functionality of
 * reassembling frames from fragments. Fragmentation reassembly classes
 * are typically instantiated as part of an UpperMac.
 */
class INET_API IReassembly
{
    public:
        virtual ~IReassembly() { }

        /**
         * Add a fragment to the reassembly buffer. If the new fragment completes a frame,
         * then the reassembled frame is returned (and fragments are removed from the buffer),
         * otherwise the function returns nullptr.
         */
        virtual Packet *addFragment(Packet *frame) = 0;

        /**
         * Discard fragments from the reassembly buffer. Frames are identified by the transmitter
         * address, the TID, and the sequence number range [startSeqNumber, endSeqNumber[.
         * Set tid=-1 for non-QoS frames.
         */
        virtual void purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // #ifndef __INET_IREASSEMBLY_H
