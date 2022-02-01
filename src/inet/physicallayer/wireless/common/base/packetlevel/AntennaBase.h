//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ANTENNABASE_H
#define __INET_ANTENNABASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntenna.h"

namespace inet {

namespace physicallayer {

class INET_API AntennaBase : public IAntenna, public cModule
{
  protected:
    opp_component_ptr<IMobility> mobility;
    int numAntennas;

  protected:
    virtual void initialize(int stage) override;

  public:
    AntennaBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual bool isDirectional() const override { return getGain()->getMinGain() != 1 || getGain()->getMaxGain() != 1; }
    virtual IMobility *getMobility() const override { return mobility; }
    virtual int getNumAntennas() const override { return numAntennas; }
};

} // namespace physicallayer

} // namespace inet

#endif

