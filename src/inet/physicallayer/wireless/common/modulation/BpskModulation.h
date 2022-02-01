//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BPSKMODULATION_H
#define __INET_BPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements binary phase-shift keying.
 *
 * http://en.wikipedia.org/wiki/Phase-shift_keying#Binary_phase-shift_keying_.28BPSK.29
 */
class INET_API BpskModulation : public MqamModulationBase
{
  public:
    static const BpskModulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    BpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "BpskModulation"; }

    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

