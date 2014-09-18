#ifndef __INET_BREAKPOINTPATHLOSS_H
#define __INET_BREAKPOINTPATHLOSS_H

#include "inet/physicallayer/contract/IPathLoss.h"

namespace inet {

namespace physicallayer {

/**
 * Implementation of a breakpoint path loss model.
 */
class INET_API BreakpointPathLoss : public cModule, public IPathLoss
{
  protected:
    /** @brief initial path loss */
    double l01, l02;
    /** @brief pathloss exponents */
    double alpha1, alpha2;
    /** @brief Breakpoint distance squared. */
    m breakpointDistance;

  protected:
    virtual void initialize(int stage);

  public:
	BreakpointPathLoss();
	virtual void printToStream(std::ostream& stream) const;
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const { return m(NaN); }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_BREAKPOINTPATHLOSS_H
