//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMITTERANALOGMODEL_H
#define __INET_UNITDISKTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/UnitDiskTransmissionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    m communicationRange = m(NaN);
    m interferenceRange = m(NaN);
    m detectionRange = m(NaN);

  protected:
    virtual void initialize(int stage) override {
        if (stage == INITSTAGE_LOCAL) {
            communicationRange = m(par("communicationRange"));
            interferenceRange = m(par("interferenceRange"));
            detectionRange = m(par("detectionRange"));
        }
    }

  public:
    virtual m getCommunicationRange() const { return communicationRange; }
    virtual m getInterferenceRange() const { return interferenceRange; }
    virtual m getDetectionRange() const { return detectionRange; }

    virtual INewTransmissionAnalogModel *createAnalogModel(const Packet *packet) const override {
        return new UnitDiskTransmissionAnalogModel(communicationRange, interferenceRange, detectionRange);
    }
};

} // namespace physicallayer

} // namespace inet

#endif

