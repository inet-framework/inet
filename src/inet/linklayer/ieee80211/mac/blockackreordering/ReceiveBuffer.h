//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVEBUFFER_H
#define __INET_RECEIVEBUFFER_H

#include <map>
#include <set>

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
    enum class FrameInsertionResult {
        INSERTED,
        REJECTED,
        REJECTED_EXPIRED
    };
    static bool isComplete(const Fragments& fragments);

  protected:
    struct BufferEntry {
        simtime_t receptionStartTime;
        bool hasFragmentedIdentity;
        bool receiveLifetimeActive;
    };

    ReorderBuffer buffer;
    std::map<SequenceNumber, BufferEntry> bufferEntries;
    std::set<SequenceNumber> expiredFragmentSequences;
    // For each Block Ack agreement, the recipient maintains a MAC variable NextExpectedSequenceNumber. The
    // NextExpectedSequenceNumber is initialized to to the value of the Starting Block Ack Starting Sequence
    // Control field of the ADDBA Request frame of the accepted Block Ack agreement. (IEEE 802.11­-11/0381r0)
    int bufferSize = -1;
    int length = 0;
    SequenceNumberCyclic nextExpectedSequenceNumber;

    void pruneExpiredFragmentSequences();

  public:
    ReceiveBuffer(int bufferSize, SequenceNumberCyclic nextExpectedSequenceNumber);
    virtual ~ReceiveBuffer();

    FrameInsertionResult insertFrameWithResult(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader);
    bool insertFrame(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader) { return insertFrameWithResult(dataPacket, dataHeader) == FrameInsertionResult::INSERTED; }
    void dropFramesUntil(SequenceNumberCyclic sequenceNumber);
    void removeFrame(SequenceNumberCyclic sequenceNumber);
    Fragments extractFrames();
    simtime_t getNextExpirationTime(simtime_t maxReceiveLifetime) const;
    Fragments removeExpiredFragments(simtime_t currentTime, simtime_t maxReceiveLifetime);

    const ReorderBuffer& getBuffer() { return buffer; }
    int getLength() { return length; }
    int getBufferSize() { return bufferSize; }
    SequenceNumberCyclic getNextExpectedSequenceNumber() { return nextExpectedSequenceNumber; }
    void setNextExpectedSequenceNumber(SequenceNumberCyclic nextExpectedSequenceNumber);
    bool isFull() { ASSERT(length <= bufferSize); return length == bufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
