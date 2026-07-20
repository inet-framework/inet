//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Attached to a packet read from a capture file whose link-layer frame still
// carries its Frame Check Sequence in the packet bytes (for example a radiotap
// 802.11 capture with the "FCS at end" flag set). Consumers that need a frame
// with an FCS trailer can use this to tell whether one must still be appended.
//
class FcsInd extends TagBase
{
}
