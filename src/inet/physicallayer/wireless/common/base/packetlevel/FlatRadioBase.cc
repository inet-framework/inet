//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmitterBase.h"

namespace inet {

namespace physicallayer {

Define_Module(FlatRadioBase);

FlatRadioBase::FlatRadioBase() :
    NarrowbandRadioBase()
{
}

void FlatRadioBase::setPower(W newPower)
{
    FlatTransmitterBase *flatTransmitter = const_cast<FlatTransmitterBase *>(check_and_cast<const FlatTransmitterBase *>(transmitter));
    flatTransmitter->setPower(newPower);
}

void FlatRadioBase::setBitrate(bps newBitrate)
{
    FlatTransmitterBase *flatTransmitter = const_cast<FlatTransmitterBase *>(check_and_cast<const FlatTransmitterBase *>(transmitter));
    flatTransmitter->setBitrate(newBitrate);
    receptionTimer = nullptr;
}

} // namespace physicallayer

} // namespace inet

