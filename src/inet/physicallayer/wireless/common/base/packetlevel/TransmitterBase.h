//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSMITTERBASE_H
#define __INET_TRANSMITTERBASE_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"

namespace inet {
namespace physicallayer {

class INET_API TransmitterBase : public Module, public virtual ITransmitter
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    virtual ITransmitterAnalogModel *getAnalogModel() const { return check_and_cast<ITransmitterAnalogModel *>(getSubmodule("analogModel")); }

    virtual W getMaxPower() const override { return W(NaN); }
    virtual m getMaxCommunicationRange() const override { return m(NaN); }
    virtual m getMaxInterferenceRange() const override { return m(NaN); }
};

} // namespace physicallayer
} // namespace inet

#endif

