//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DBPSKMODULATION_H
#define __INET_DBPSKMODULATION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/DpskModulationBase.h"

namespace inet {

namespace physicallayer {

class INET_API DbpskModulation : public DpskModulationBase
{
  public:
    static const DbpskModulation singleton;

  public:
    DbpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "DbpskModulation"; }
};

} // namespace physicallayer

} // namespace inet

#endif

