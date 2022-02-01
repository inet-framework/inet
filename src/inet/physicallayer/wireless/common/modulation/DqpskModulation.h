//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DQPSKMODULATION_H
#define __INET_DQPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DpskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API DqpskModulation : public DpskModulationBase
{
  public:
    static const DqpskModulation singleton;

  public:
    DqpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "DqpskModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

