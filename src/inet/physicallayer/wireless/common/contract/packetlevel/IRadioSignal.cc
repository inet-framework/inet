//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

cEnum *IRadioSignal::signalPartEnum = nullptr;

Register_Enum(inet::physicallayer::IRadioSignal::SignalPart,
    (IRadioSignal::SIGNAL_PART_NONE,
     IRadioSignal::SIGNAL_PART_WHOLE,
     IRadioSignal::SIGNAL_PART_PREAMBLE,
     IRadioSignal::SIGNAL_PART_HEADER,
     IRadioSignal::SIGNAL_PART_DATA));

const char *IRadioSignal::getSignalPartName(SignalPart signalPart)
{
    if (signalPartEnum == nullptr)
        signalPartEnum = cEnum::get(opp_typename(typeid(IRadioSignal::SignalPart)));
    return signalPartEnum->getStringFor(signalPart) + 12;
}

} // namespace physicallayer

} // namespace inet

