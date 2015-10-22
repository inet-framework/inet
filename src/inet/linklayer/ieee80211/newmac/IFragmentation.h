//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_IFRAGMENTATION_H
#define __INET_IFRAGMENTATION_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtFrame;

/**
 * Abstract interface for classes that encapsulate the functionality of
 * fragmenting frames. Fragmentation classes are typically instantiated
 * as part of an UpperMac.
 */
class INET_API IFragmenter
{
    public:
        /**
         * Splits up the frame, and returns the fragments in a vector. The decision whether
         * a given frame needs to be fragmented or not is outside the scope of this class.
         */
        virtual std::vector<Ieee80211DataOrMgmtFrame*> fragment(Ieee80211DataOrMgmtFrame *frame, int fragmentationThreshold) = 0;
        virtual ~IFragmenter() {}
};

/**
 * Abstract interface for classes that encapsulate the functionality of
 * reassembling frames from fragments. Fragmentation reassembly classes
 * are typically instantiated as part of an UpperMac.
 */
class INET_API IReassembly
{
    public:
        /**
         * Add a fragment to the reassembly buffer. If the new fragment completes a frame,
         * then the reassembled frame is returned (and fragments are removed from the buffer),
         * otherwise the function returns nullptr.
         */
        virtual Ieee80211DataOrMgmtFrame *addFragment(Ieee80211DataOrMgmtFrame *frame) = 0;

        /**
         * Discard fragments from the reassembly buffer. Frames are identified by the transmitter
         * address, the TID, and the sequence number range [startSeqNumber, endSeqNumber[.
         * Set tid=-1 for non-QoS frames.
         */
        virtual void purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber) = 0;

        virtual ~IReassembly() {}
};

} // namespace ieee80211
} // namespace inet

#endif
