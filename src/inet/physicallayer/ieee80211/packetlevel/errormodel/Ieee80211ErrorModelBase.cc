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

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    const ITransmission *transmission = snir->getReception()->getTransmission();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    const Ieee80211TransmissionBase *ieee80211Transmission = check_and_cast<const Ieee80211TransmissionBase *>(transmission);
    const IIeee80211Mode *mode = ieee80211Transmission->getMode();
    double headerSuccessRate = getHeaderSuccessRate(mode, flatTransmission->getHeaderBitLength(), snir->getMin());
    double dataSuccessRate = getDataSuccessRate(mode, flatTransmission->getDataBitLength(), snir->getMin());
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

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISNIR *snir, IRadioSignal::SignalPart part) const
{
    Enter_Method_Silent();
    return NaN;
}

} // namespace physicallayer

} // namespace inet

