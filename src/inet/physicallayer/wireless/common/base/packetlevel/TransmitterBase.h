//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSMITTERBASE_H
#define __INET_TRANSMITTERBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"

namespace inet {
namespace physicallayer {

class INET_API TransmitterBase : public cModule, public virtual ITransmitter
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    virtual W getMaxPower() const override { return W(NaN); }
    virtual m getMaxCommunicationRange() const override { return m(NaN); }
    virtual m getMaxInterferenceRange() const override { return m(NaN); }
};

} // namespace physicallayer
} // namespace inet

#endif

