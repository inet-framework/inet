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

#include <algorithm>

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

uint16_t BabelCostEtx::computeRxcost(uint16_t history, uint16_t nominalrxcost)
{
    // rate the history: the last 3 intervals count fully, older ones with decreasing weight
    unsigned int histrate = ((history & 0x8000) >> 2) + ((history & 0x4000) >> 1) + (history & 0x3FFF);
    // with no loss the divisor is 0x8000, giving rxcost == nominalrxcost
    unsigned int rxcost = (0x8000u * nominalrxcost) / (histrate + 1);
    return (rxcost >= COST_INF) ? COST_INF : static_cast<uint16_t>(rxcost & 0xFFFF);
}

uint16_t BabelCostEtx::computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost)
{
    uint16_t rxcost = computeRxcost(history, nominalrxcost);
    if (rxcost >= COST_INF)
        return COST_INF;
    if (txcost < 256 && rxcost < 256)
        return txcost; // both directions better than 100% delivery
    unsigned int a = std::max<uint16_t>(txcost, 256);
    unsigned int b = std::max<uint16_t>(rxcost, 256);
    return static_cast<uint16_t>((a * b) >> 8);
}

} // namespace babel
} // namespace inet
