//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRANSMITTERANALOGMODEL_H
#define __INET_ITRANSMITTERANALOGMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {

namespace physicallayer {

/**
 * Implementors are responsible for creating the analog representation of
 * transmissions emitted by the radio, to be propagated through the medium analog model.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API ITransmitterAnalogModel : public IPrintableObject
{
  public:
    virtual ITransmissionAnalogModel *createAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth, W power) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

