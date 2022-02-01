//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QPSKMODULATION_H
#define __INET_QPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements quadrature phase-shift keying.
 *
 * http://en.wikipedia.org/wiki/Phase-shift_keying#Quadrature_phase-shift_keying_.28QPSK.29
 */
class INET_API QpskModulation : public MqamModulationBase
{
  public:
    static const QpskModulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    QpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "QpskModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

