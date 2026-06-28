//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/babel/BabelCost.h"

#include "inet/routing/babel/BabelDefs.h"

namespace inet {
namespace babel {

uint16_t BabelCostKoutofj::computeRxcost(uint16_t history, uint16_t nominalrxcost)
{
    ASSERT(k > 0);
    ASSERT(j >= k);
    ASSERT(nominalrxcost >= 1);

    unsigned int reccount = 0;
    for (unsigned int i = 0; i < j; ++i)
        if ((history << i) & 0x8000)
            ++reccount;

    return reccount >= k ? nominalrxcost : COST_INF;
}

uint16_t BabelCostKoutofj::computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost)
{
    // if the link is usable in the receive direction, the cost is the txcost reported by the neighbour
    return computeRxcost(history, nominalrxcost) != COST_INF ? txcost : COST_INF;
}

} // namespace babel
} // namespace inet
