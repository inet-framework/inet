//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_RECEIVEBUFFER_H
#define __INET_RECEIVEBUFFER_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"

namespace inet {
namespace ieee80211 {

class INET_API ReceiveBuffer
{
    public:
        typedef std::vector<Packet *> Fragments;
        typedef std::map<SequenceNumber, Fragments> ReorderBuffer;

    protected:
        ReorderBuffer buffer;
        // For each Block Ack agreement, the recipient maintains a MAC variable NextExpectedSequenceNumber. The
        // NextExpectedSequenceNumber is initialized to to the value of the Starting Block Ack Starting Sequence
        // Control field of the ADDBA Request frame of the accepted Block Ack agreement. (IEEE 802.11­-11/0381r0)
        int bufferSize = -1;
        int length = 0;
        SequenceNumber nextExpectedSequenceNumber = -1;

    public:
        ReceiveBuffer(int bufferSize, SequenceNumber nextExpectedSequenceNumber);
        virtual ~ReceiveBuffer();

        bool insertFrame(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader);
        void dropFramesUntil(SequenceNumber sequenceNumber);
        void removeFrame(SequenceNumber sequenceNumber);

        const ReorderBuffer& getBuffer() { return buffer; }
        int getLength() { return length; }
        int getBufferSize() { return bufferSize; }
        SequenceNumber getNextExpectedSequenceNumber() { return nextExpectedSequenceNumber; }
        void setNextExpectedSequenceNumber(SequenceNumber nextExpectedSequenceNumber) { this->nextExpectedSequenceNumber = nextExpectedSequenceNumber; }
        bool isFull() { ASSERT(length <= bufferSize); return length == bufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

