//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROPAGATIONBASE_H
#define __INET_PROPAGATIONBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IPropagation.h"

namespace inet {

namespace physicallayer {

class INET_API PropagationBase : public cModule, public IPropagation
{
  protected:
    mps propagationSpeed;
    mutable long arrivalComputationCount;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    PropagationBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual mps getPropagationSpeed() const override { return propagationSpeed; }
};

} // namespace physicallayer

} // namespace inet

#endif

