//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRANSMITTERANALOGMODEL_H
#define __INET_ITRANSMITTERANALOGMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewTransmissionAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API ITransmitterAnalogModel : public IPrintableObject
{
  public:
    virtual INewTransmissionAnalogModel *createAnalogModel(const Packet *packet, simtime_t duration, Hz centerFrequency, Hz bandwidth, W power) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

