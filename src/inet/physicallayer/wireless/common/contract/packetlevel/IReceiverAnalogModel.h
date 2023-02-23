//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECEIVERANALOGMODEL_H
#define __INET_IRECEIVERANALOGMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

/**
 * This class represents a receiver analog model. It is the connecting
 * piece between the technology-dependent part of the receivers, and
 * the different analog medium models.
 */
class INET_API IReceiverAnalogModel : public IPrintableObject
{
  public:
    /// Factory method for IListening objects.
    virtual IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, Hz centerFrequency, Hz bandwidth) const = 0;

    /// Returns false if the analog model of the receiver considers the reception impossible.
    /// A true return value does not necessarily mean that the reception is possible,
    /// just that it is not impossible according to the analog model part of the receiver.
    /// All it does is check the minimum incoming power level against a given sensitivity threshold.
    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, W sensitivity) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

