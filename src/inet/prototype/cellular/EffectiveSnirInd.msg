//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet::cellular;

//
// Carries the effective SNIR of a received transport block, which the MACs use for link
// adaptation.
//
class EffectiveSnirInd extends TagBase
{
    double effectiveSnir = NaN; // Effective SNIR over the cells of the allocation, as a ratio
}
