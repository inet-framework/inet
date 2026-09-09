//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::ieee80211;

// Identifies one local originator ADDBA transaction across packet transformations.
class Ieee80211AddbaTransactionTag extends TagBase
{
    uint64_t transactionId;
}
