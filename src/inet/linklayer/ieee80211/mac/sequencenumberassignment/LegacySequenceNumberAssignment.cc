//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/sequencenumberassignment/LegacySequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

//
// A non-QoS STA shall assign sequence numbers to management frames and data frames (QoS subfield of the
// Subtype field is equal to 0) from a single modulo-4096 counter, starting at 0 and incrementing by 1, for each
// MSDU or MMPDU.
//
void LegacySequenceNumberAssignment::assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header)
{
    ASSERT(header->getType() != ST_DATA_WITH_QOS);
    lastSeqNum = lastSeqNum + 1;
    header->setSequenceNumber(lastSeqNum);
}

} /* namespace ieee80211 */
} /* namespace inet */

