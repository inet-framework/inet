//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BLOCKACKRECORD_H
#define __INET_BLOCKACKRECORD_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// The recipient shall maintain a Block Ack record consisting of originator address, TID, and a record of
// reordering buffer size indexed by the received MPDU sequence control value. This record holds the
// acknowledgment state of the data frames received from the originator.
//
class INET_API BlockAckRecord
{
  protected:
    MacAddress originatorAddress = MacAddress::UNSPECIFIED_ADDRESS;
    Tid tid = -1;
    SequenceNumberCyclic startingSequenceNumber;
    int windowSize = 64;
    std::map<SequenceControlField, bool> acknowledgmentState;

  public:
    BlockAckRecord(MacAddress originatorAddress, Tid tid, SequenceNumberCyclic startingSequenceNumber = SequenceNumberCyclic(0), int bufferSize = 64);
    virtual ~BlockAckRecord() {}

    void blockAckPolicyFrameReceived(const Ptr<const Ieee80211DataHeader>& header);
    bool getAckState(SequenceNumberCyclic sequenceNumber, FragmentNumber fragmentNumber);
    void removeAckStates(SequenceNumberCyclic sequenceNumber);
    void blockAckReqReceived(SequenceNumberCyclic sequenceNumber);
    SequenceNumberCyclic getStartingSequenceNumber() const { return startingSequenceNumber; }
    int getWindowSize() const { return windowSize; }

    MacAddress getOriginatorAddress() { return originatorAddress; }
    Tid getTid() { return tid; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif
