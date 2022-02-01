//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockackreordering/ReceiveBuffer.h"

namespace inet {
namespace ieee80211 {

ReceiveBuffer::ReceiveBuffer(int bufferSize, SequenceNumberCyclic nextExpectedSequenceNumber) :
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
bool ReceiveBuffer::insertFrame(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader)
{
    auto sequenceNumber = dataHeader->getSequenceNumber();
    auto fragmentNumber = dataHeader->getFragmentNumber();
    // The total number of MPDUs in these MSDUs may not
    // exceed the reorder buffer size in the receiver.
    if (length < bufferSize && nextExpectedSequenceNumber <= sequenceNumber && sequenceNumber < nextExpectedSequenceNumber + bufferSize) {
        auto it = buffer.find(sequenceNumber.get());
        if (it != buffer.end()) {
            auto& fragments = it->second;
            // TODO efficiency
            for (auto fragment : fragments) {
                const auto& fragmentHeader = fragment->peekAtFront<Ieee80211DataHeader>();
                if (fragmentHeader->getSequenceNumber() == sequenceNumber && fragmentHeader->getFragmentNumber() == fragmentNumber)
                    return false;
            }
            fragments.push_back(dataPacket);
        }
        else {
            buffer[sequenceNumber.get()].push_back(dataPacket);
        }
        // The total number of frames that can be sent depends on the total
        // number of MPDUs in all the outstanding MSDUs.
        length++;
        return true;
    }
    return false;
}

void ReceiveBuffer::dropFramesUntil(SequenceNumberCyclic sequenceNumber)
{
    auto it = buffer.begin();
    while (it != buffer.end()) {
        if (SequenceNumberCyclic(it->first) < sequenceNumber) {
            length -= it->second.size();
            for (auto fragment : it->second)
                delete fragment;
            it = buffer.erase(it);
        }
        else
            it++;
    }
}

void ReceiveBuffer::removeFrame(SequenceNumberCyclic sequenceNumber)
{
    auto it = buffer.find(sequenceNumber.get());
    if (it != buffer.end()) {
        length -= it->second.size();
        buffer.erase(sequenceNumber.get());
    }
    else
        throw cRuntimeError("Unknown sequence number: %d", sequenceNumber.get());
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

