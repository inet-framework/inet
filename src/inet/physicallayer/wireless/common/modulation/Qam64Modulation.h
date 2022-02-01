//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QAM64MODULATION_H
#define __INET_QAM64MODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements gray coded rectangular quadrature amplitude
 * modulation with 64 symbols.
 *
 * http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation
 */
class INET_API Qam64Modulation : public MqamModulationBase
{
  public:
    static const Qam64Modulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    Qam64Modulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Qam64Modulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

