//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCALARTRANSMITTERANALOGMODEL_H
#define __INET_SCALARTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewTransmissionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ScalarTransmissionAnalogModel : public INewTransmissionAnalogModel
{
  public:
    Hz centerFrequency = Hz(NaN);
    Hz bandwidth = Hz(NaN);
    W power = W(NaN);

  public:
    ScalarTransmissionAnalogModel(Hz centerFrequency, Hz bandwidth, W power) : centerFrequency(centerFrequency), bandwidth(bandwidth), power(power) {}
};

class INET_API ScalarTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    Hz centerFrequency = Hz(NaN);
    Hz bandwidth = Hz(NaN);
    W power = W(NaN);

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
            centerFrequency = Hz(par("centerFrequency"));
            bandwidth = Hz(par("bandwidth"));
            power = W(par("power"));
        }
    }

  public:
    virtual INewTransmissionAnalogModel *createAnalogModel(const Packet *packet) const override {
        return new ScalarTransmissionAnalogModel(centerFrequency, bandwidth, power);
    }
};

} // namespace physicallayer

} // namespace inet

#endif

