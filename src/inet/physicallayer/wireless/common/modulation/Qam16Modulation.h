//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QAM16MODULATION_H
#define __INET_QAM16MODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements gray coded rectangular quadrature amplitude
 * modulation with 16 symbols.
 *
 * http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation
 */
class INET_API Qam16Modulation : public MqamModulationBase
{
  public:
    static const Qam16Modulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    Qam16Modulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Qam16Modulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

