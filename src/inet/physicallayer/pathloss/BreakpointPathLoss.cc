#include "inet/physicallayer/pathloss/BreakpointPathLoss.h"

namespace inet {

namespace physicallayer {

Define_Module(BreakpointPathLoss);

BreakpointPathLoss::BreakpointPathLoss() :
    l01(sNaN),
    l02(sNaN),
    alpha1(sNaN),
    alpha2(sNaN),
    breakpointDistance(m(sNaN))
{
}

void BreakpointPathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        l01 = math::dB2fraction(par("L01"));
        l02 = math::dB2fraction(par("L02"));
        alpha1 = par("alpha1");
        alpha2 = par("alpha2");
        breakpointDistance = m(par("breakpointDistance"));
    }
}

void BreakpointPathLoss::printToStream(std::ostream& stream) const
{
    stream << "BreakpointPathLoss, "
           << "L01 = " << l01 << ", "
           << "L02 = " << l02 << ", "
           << "alpha1 = " << alpha1 << ", "
           << "alpha2 = " << alpha2 << ", "
           << "breakpointDistance = " << breakpointDistance;
}

double BreakpointPathLoss::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    // PL(d) = PL0 + 10 alpha log10 (d/d0)
    // 10 ^ { PL(d)/10 } = 10 ^{PL0 + 10 alpha log10 (d/d0)}/10
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { 10 log10 (d/d0)^alpha }/10
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * 10 ^ { log10 (d/d0)^alpha }
    // 10 ^ { PL(d)/10 } = 10 ^ PL0/10 * (d/d0)^alpha
    if (distance < breakpointDistance)
        return 1 / (l01 * pow(distance.get(), alpha1));
    else
        return 1 / (l02 * pow(unit(distance / breakpointDistance).get(), alpha2));
}

} // namespace physicallayer

} // namespace inet
