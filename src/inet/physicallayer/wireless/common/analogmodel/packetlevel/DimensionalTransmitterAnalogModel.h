//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DIMENSIONALTRANSMITTERANALOGMODEL_H
#define __INET_DIMENSIONALTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmissionAnalogModel.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewTransmissionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API DimensionalTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    const Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> powerFunction;

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
        }
    }

  public:
    virtual INewTransmissionAnalogModel *createAnalogModel(const Packet *packet) const override {
        return new DimensionalTransmissionAnalogModel(powerFunction);
    }
};

} // namespace physicallayer

} // namespace inet

#endif

