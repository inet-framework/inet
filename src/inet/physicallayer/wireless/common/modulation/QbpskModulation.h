//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QBPSKMODULATION_H
#define __INET_QBPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/MqamModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This class implements quadrature binary phase-shift keying defined
 * in 20.3.9.4.3 HT-SIG definition.
 */
class INET_API QbpskModulation : public MqamModulationBase
{
  public:
    static const QbpskModulation singleton;

  protected:
    static const std::vector<ApskSymbol> constellation;

  public:
    QbpskModulation();

    virtual void printToStream(std::ostream& stream) const { stream << "QbpskModulation"; }

    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

