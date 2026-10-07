//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TWORAYINTERFERENCE_H
#define __INET_TWORAYINTERFERENCE_H

#include "inet/common/Module.h"
#include <inet/physicallayer/wireless/common/contract/packetlevel/IPathLoss.h>

namespace inet {
namespace physicallayer {

/**
 * Two-Ray interference model borrowed from Veins (default parameterization)
 *
 * See "Using the Right Two-Way Model? A Measurement-based Evaluation of PHY Models in VANETs"
 * by C. Sommer and F. Dressler (2011)
 * or "On the Applicability of Two-Ray Path Loss Models for Vehicular Network Simulation"
 * by C. Sommer, S. Joerer and F. Dressler (2012)
 */
class INET_API TwoRayInterference : public Module, public IPathLoss
{
  public:
    TwoRayInterference();
    void initialize(int stage) override;
    Ptr<const math::IFunction<double, math::Domain<Hz>>> computeReceptionPathLoss(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const override;
    double computePathLoss(mps propagation, Hz frequency, m distance) const override;
    m computeRange(mps propagation, Hz frequency, double loss) const override;
    std::ostream& printToStream(std::ostream&, int level, int evFlags = 0) const override;

  protected:
    class TwoRayInterferenceFunction;

  protected:
    double epsilon_r;
    char polarization;

  protected:
    virtual double computeTwoRayInterference(const Coord& posTx, const Coord& posRx, m waveLength) const;
    virtual double reflectionCoefficient(double cos_theta, double sin_theta) const;
};

} // namespace physicallayer
} // namespace inet

#endif

