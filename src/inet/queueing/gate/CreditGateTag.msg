//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// This tag carries the module id of the ~CreditBasedGate which the packet passed
// through.
//
// This tag is part of the credit based shaper infrastructure.
//
// @see ~CreditBasedGate
//
class CreditGateTag extends TagBase
{
    int id = -1; // module id of the ~CreditBasedGate which the packet passed through
}
