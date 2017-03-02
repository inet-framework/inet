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

#include "inet/linklayer/ieee80211/mac/framesequence/HtTxOpFs.h"
#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

HtTxOpFs::HtTxOpFs() :
    // G.4 HT sequences
    // ht-txop-sequence =
    //   L-sig-protected-sequence |
    //   ht-nav-protected-sequence |
    //   dual-cts-protected-sequence |
    //   1 {initiator-sequence};
    AlternativesFs({},
                   ALTERNATIVESFS_SELECTOR(selectHtTxOpSequence))
{
}

int HtTxOpFs::selectHtTxOpSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return 0;
}

} // namespace ieee80211
} // namespace inet

