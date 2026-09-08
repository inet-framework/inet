//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::ieee80211;

//
// Identifies a local management transaction across packet replacement and
// fragmentation. The physical layer removes this sender-local metadata.
//
class Ieee80211MgmtTransactionTag extends TagBase
{
    uint64_t transactionId = 0;
}
