//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DSSSOQPSK16MODULATION_H
#define __INET_DSSSOQPSK16MODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API DsssOqpsk16Modulation : public ApskModulationBase
{
  public:
    static const DsssOqpsk16Modulation singleton;

  public:
    DsssOqpsk16Modulation();
    virtual ~DsssOqpsk16Modulation();

    double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

