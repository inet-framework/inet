//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MQAMMODULATIONBASE_H
#define __INET_MQAMMODULATIONBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * Base class for gray coded rectangular quadrature amplitude modulations.
 */
class INET_API MqamModulationBase : public ApskModulationBase
{
  protected:
    double normalizationFactor = NaN;

  public:
    MqamModulationBase(double normalizationFactor, const std::vector<ApskSymbol> *constellation);

    virtual double getNormalizationFactor() const { return normalizationFactor; }

    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

