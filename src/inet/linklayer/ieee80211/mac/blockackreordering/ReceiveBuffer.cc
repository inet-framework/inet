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

#include "ReceiveBuffer.h"

namespace inet {
namespace ieee80211 {

ReceiveBuffer::ReceiveBuffer(int bufferSize, int nextExpectedSequenceNumber) :
        bufferSize(bufferSize),
        nextExpectedSequenceNumber(nextExpectedSequenceNumber)
{
}

//
// Upon the receipt of a QoS data frame from the originator for which a Block Ack agreement exists, the recipient
// buffers the MSDU regardless of the value of the Ack Policy subfield within the QoS Control field of the QoS
// data frame, unless the sequence number of the frame is older than the NextExpectedSequenceNumber for that
// Block Ack agreement, in which case the frame is discarded because it is either old or a duplicate.
//
bool ReceiveBuffer::insertFrame(Ieee80211DataFrame* dataFrame)
{
    int sequenceNumber = dataFrame->getSequenceNumber();
    // The total number of MPDUs in these MSDUs may not
    // exceed the reorder buffer size in the receiver.
    if (length < bufferSize && !isSequenceNumberTooOld(sequenceNumber, nextExpectedSequenceNumber, bufferSize)) {
        auto it = buffer.find(sequenceNumber);
        if (it != buffer.end()) {
            auto &fragments = it->second;
            fragments.push_back(dataFrame);
        }
        else {
            buffer[sequenceNumber].push_back(dataFrame);
        }
        // The total number of frames that can be sent depends on the total
        // number of MPDUs in all the outstanding MSDUs.
        length++;
        return true;
    }
    return false;
}

void ReceiveBuffer::remove(int sequenceNumber)
{
    auto it = buffer.find(sequenceNumber);
    if (it != buffer.end()) {
        length -= it->second.size();
        buffer.erase(sequenceNumber);
    }
    else
        throw cRuntimeError("Unknown sequence number = %d", sequenceNumber);
}

ReceiveBuffer::~ReceiveBuffer()
{
    for (auto fragments : buffer) {
        for (auto fragment : fragments.second)
            delete fragment;
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
