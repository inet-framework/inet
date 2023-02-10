//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARTRANSMITTERANALOGMODEL_H
#define __INET_SCALARTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmissionAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewTransmissionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    Hz defaultCenterFrequency = Hz(NaN);
    Hz defaultBandwidth = Hz(NaN);
    W defaultPower = W(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual INewTransmissionAnalogModel *createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

