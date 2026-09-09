//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::ieee80211;

// Identifies one local Block Ack agreement generation across packet transformations.
// This metadata is not transmitted as part of the DELBA frame.
class Ieee80211BlockAckAgreementTag extends TagBase
{
    uint64_t generationId;
}
