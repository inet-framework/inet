#include "inet/physicallayer/pathloss/BreakpointPathLoss.h"

namespace inet {

namespace physicallayer {

Define_Module(BreakpointPathLoss);

BreakpointPathLoss::BreakpointPathLoss() :
    l01(NaN),
    l02(NaN),
    alpha1(NaN),
    alpha2(NaN),
    breakpointDistance(m(NaN))
{
}

void BreakpointPathLoss::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        l01 = math::dB2fraction(par("l01"));
        l02 = math::dB2fraction(par("l02"));
        alpha1 = par("alpha1");
        alpha2 = par("alpha2");
        breakpointDistance = m(par("breakpointDistance"));
    }
}

std::ostream& BreakpointPathLoss::printToStream(std::ostream& stream, int level) const
{
    stream << "BreakpointPathLoss";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", L01 = " << l01
               << ", L02 = " << l02
               << ", alpha1 = " << alpha1
               << ", alpha2 = " << alpha2
               << ", breakpointDistance = " << breakpointDistance;
    return stream;
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

m BreakpointPathLoss::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    // this assumes the path loss is a continuous function
    // the result should be the same for the second case including l02 and alpha2
    double breakpointPathLoss = 1 / (l01 * pow(breakpointDistance.get(), alpha1));

    if(loss < breakpointPathLoss) {
        // the allowed loss factor is smaller than the one faced at the breakpoint distance
        // -> range is higher than breakpointDistance
        // loss = 1 / (l02 * (d / d0)^alpha2)
        // (d / d0)^alpha2 = 1 / (loss * l02)
        // (d / d0) = 1 / ((loss * l02)^(1/alpha2))
        // d = d0 / ((loss * l02)^(1/alpha2))
        return breakpointDistance / (pow(loss * l02, 1/alpha2));
    }
    else {
        return m(1) / (pow(loss * l01, 1/alpha1));
    }
}

} // namespace physicallayer

} // namespace inet
