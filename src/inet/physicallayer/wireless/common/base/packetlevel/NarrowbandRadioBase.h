//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDRADIOBASE_H
#define __INET_NARROWBANDRADIOBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandRadioBase : public Radio
{
  protected:
    void handleUpperCommand(cMessage *message) override;

  public:
    NarrowbandRadioBase();

    virtual void setModulation(const IModulation *newModulation);
    virtual void setCenterFrequency(Hz newCenterFrequency);
    virtual void setBandwidth(Hz newBandwidth);
};

} // namespace physicallayer

} // namespace inet

#endif

