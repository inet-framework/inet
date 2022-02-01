//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INTERFERENCE_H
#define __INET_INTERFERENCE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IInterference.h"

namespace inet {
namespace physicallayer {

class INET_API Interference : public virtual IInterference
{
  protected:
    const INoise *backgroundNoise;
    const std::vector<const IReception *> *interferingReceptions;

  public:
    Interference(const INoise *noise, const std::vector<const IReception *> *receptions);
    virtual ~Interference();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const INoise *getBackgroundNoise() const override { return backgroundNoise; }
    virtual const std::vector<const IReception *> *getInterferingReceptions() const override { return interferingReceptions; }
};

} // namespace physicallayer
} // namespace inet

#endif

