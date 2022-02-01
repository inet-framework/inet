//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.h"

#include "inet/physicallayer/wireless/common/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

Ieee80211ErrorModelBase::Ieee80211ErrorModelBase()
{
}

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computePacketErrorRate");
    auto transmission = snir->getReception()->getTransmission();
    auto flatTransmission = dynamic_cast<const FlatTransmissionBase *>(transmission);
    auto ieee80211Transmission = check_and_cast<const Ieee80211TransmissionBase *>(transmission);
    auto mode = ieee80211Transmission->getMode();
    auto headerLength = flatTransmission != nullptr ? flatTransmission->getHeaderLength() : mode->getHeaderMode()->getLength();
    auto dataLength = flatTransmission != nullptr ? flatTransmission->getDataLength() : mode->getDataMode()->getCompleteLength(transmission->getPacket()->getTotalLength() - headerLength);
    // TODO check header length and data length for OFDM (signal) field
    double headerSuccessRate = getHeaderSuccessRate(mode, b(headerLength).get(), getScalarSnir(snir));
    double dataSuccessRate = getDataSuccessRate(mode, b(dataLength).get(), getScalarSnir(snir));
    switch (part) {
        case IRadioSignal::SIGNAL_PART_WHOLE:
            return 1.0 - headerSuccessRate * dataSuccessRate;
        case IRadioSignal::SIGNAL_PART_PREAMBLE:
            return 0;
        case IRadioSignal::SIGNAL_PART_HEADER:
            return 1.0 - headerSuccessRate;
        case IRadioSignal::SIGNAL_PART_DATA:
            return 1.0 - dataSuccessRate;
        default:
            throw cRuntimeError("Unknown signal part: '%s'", IRadioSignal::getSignalPartName(part));
    }
}

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeBitErrorRate");
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method("computeSymbolErrorRate");
    return NaN;
}

Packet *Ieee80211ErrorModelBase::computeCorruptedPacket(const Packet *packet, double ber) const
{
    if (corruptionMode == CorruptionMode::CM_PACKET)
        return ErrorModelBase::computeCorruptedPacket(packet, ber);
    else
        throw cRuntimeError("Unimplemented corruption mode");
}

} // namespace physicallayer

} // namespace inet

