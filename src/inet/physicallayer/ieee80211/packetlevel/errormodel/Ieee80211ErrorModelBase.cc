//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.h"

namespace inet {

namespace physicallayer {

Ieee80211ErrorModelBase::Ieee80211ErrorModelBase()
{
}

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    auto transmission = snir->getReception()->getTransmission();
    auto flatTransmission = dynamic_cast<const FlatTransmissionBase *>(transmission);
    auto ieee80211Transmission = check_and_cast<const Ieee80211TransmissionBase *>(transmission);
    auto mode = ieee80211Transmission->getMode();
    auto headerLength = flatTransmission != nullptr ? flatTransmission->getHeaderLength() : mode->getHeaderMode()->getLength();
    auto dataLength = flatTransmission != nullptr ? flatTransmission->getDataLength() : mode->getDataMode()->getCompleteLength(transmission->getPacket()->getTotalLength() - headerLength);
    // TODO: check header length and data length for OFDM (signal) field
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
    Enter_Method_Silent();
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
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

