//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    SequenceNumberCyclic nextExpectedSequenceNumber;

  public:
    ReceiveBuffer(int bufferSize, SequenceNumberCyclic nextExpectedSequenceNumber);
    virtual ~ReceiveBuffer();

    bool insertFrame(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader);
    void dropFramesUntil(SequenceNumberCyclic sequenceNumber);
    void removeFrame(SequenceNumberCyclic sequenceNumber);

    const ReorderBuffer& getBuffer() { return buffer; }
    int getLength() { return length; }
    int getBufferSize() { return bufferSize; }
    SequenceNumberCyclic getNextExpectedSequenceNumber() { return nextExpectedSequenceNumber; }
    void setNextExpectedSequenceNumber(SequenceNumberCyclic nextExpectedSequenceNumber) { this->nextExpectedSequenceNumber = nextExpectedSequenceNumber; }
    bool isFull() { ASSERT(length <= bufferSize); return length == bufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

