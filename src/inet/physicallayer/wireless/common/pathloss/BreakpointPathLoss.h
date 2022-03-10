//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BREAKPOINTPATHLOSS_H
#define __INET_BREAKPOINTPATHLOSS_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"

namespace inet {

namespace physicallayer {

/**
 * Implementation of a breakpoint path loss model.
 */
class INET_API BreakpointPathLoss : public PathLossBase
{
  protected:
    /** @brief initial path loss */
    double l01, l02;
    /** @brief pathloss exponents */
    double alpha1, alpha2;
    /** @brief Breakpoint distance squared. */
    m breakpointDistance;

  protected:
    virtual void initialize(int stage) override;

  public:
    BreakpointPathLoss();
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

