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
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewReceptionAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {
namespace physicallayer {

class INET_API IReceiverAnalogModel : public IPrintableObject
{
  public:
    virtual IListening *createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition) const = 0;

    virtual const IListeningDecision *computeListeningDecision(const IListening *listening, const IInterference *interference) const = 0;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const = 0;

    virtual bool computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

