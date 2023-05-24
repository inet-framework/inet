//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMITTERANALOGMODEL_H
#define __INET_UNITDISKTRANSMITTERANALOGMODEL_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterAnalogModelBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitterAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskTransmitterAnalogModel : public TransmitterAnalogModelBase, public ITransmitterAnalogModel
{
  protected:
    m communicationRange;
    m interferenceRange;
    m detectionRange;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual m getCommunicationRange() const { return communicationRange; }
    virtual m getInterferenceRange() const { return interferenceRange; }
    virtual m getDetectionRange() const { return detectionRange; }

    virtual ITransmissionAnalogModel* createAnalogModel(const Packet *packet, simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

