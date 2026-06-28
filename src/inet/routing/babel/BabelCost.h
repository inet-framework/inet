//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABELCOST_H
#define __INET_BABELCOST_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace babel {

/**
 * Link-cost computation strategy (RFC 6126, section 3.4.3). The history vector
 * is a uint16_t with the most recent Hello interval in the most significant bit.
 */
class INET_API IBabelCostComputation
{
  public:
    virtual ~IBabelCostComputation() {}
    virtual uint16_t computeRxcost(uint16_t history, uint16_t nominalrxcost) = 0;
    virtual uint16_t computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost) = 0;
};

/**
 * The "k-out-of-j" strategy (RFC 6126, appendix A.2.1), appropriate for wired
 * links: the link is usable (rxcost = nominal) iff at least k of the last j
 * Hellos were received, otherwise the cost is infinite.
 */
class INET_API BabelCostKoutofj : public IBabelCostComputation
{
  protected:
    unsigned int k;
    unsigned int j;

  public:
    BabelCostKoutofj(unsigned int k = 2, unsigned int j = 3) : k(k), j(j) {}
    virtual uint16_t computeRxcost(uint16_t history, uint16_t nominalrxcost) override;
    virtual uint16_t computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost) override;
};

} // namespace babel
} // namespace inet

#endif
