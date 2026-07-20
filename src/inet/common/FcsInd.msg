//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Attached to a packet read from a capture file whose link-layer FCS presence is
// known from the file itself, so consumers do not have to assume it. It is set
// only for capture formats that encode this -- a radiotap 802.11 header (the
// per-frame "FCS at end" flag) or an FCS-explicit link type such as
// DLT_IEEE802_15_4_WITHFCS / _NOFCS. For formats that do not encode it (plain
// Ethernet, raw 802.11) the tag is absent and the presence of an FCS must be
// supplied out of band. hasFcs tells whether the frame bytes still carry the FCS.
//
class FcsInd extends TagBase
{
    bool hasFcs;
}
