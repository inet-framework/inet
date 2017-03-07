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

#include "LegacySequenceNumberAssigment.h"

namespace inet {
namespace ieee80211 {

//
// A non-QoS STA shall assign sequence numbers to management frames and data frames (QoS subfield of the
// Subtype field is equal to 0) from a single modulo-4096 counter, starting at 0 and incrementing by 1, for each
// MSDU or MMPDU.
//
void LegacySequenceNumberAssigment::assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    lastSeqNum = (lastSeqNum + 1) % 4096;
    frame->setSequenceNumber(lastSeqNum);
}

} /* namespace ieee80211 */
} /* namespace inet */

