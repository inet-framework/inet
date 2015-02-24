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

#include "inet/physicallayer/base/FlatTransmissionBase.h"
#include "inet/physicallayer/ieee80211/Ieee80211TransmissionBase.h"
#include "inet/physicallayer/ieee80211/errormodel/Ieee80211ErrorModelBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"

namespace inet {

namespace physicallayer {

using namespace ieee80211;

Ieee80211ErrorModelBase::Ieee80211ErrorModelBase()
{
}

double Ieee80211ErrorModelBase::computePacketErrorRate(const ISNIR *snir) const
{
    const ITransmission *transmission = snir->getReception()->getTransmission();
    const FlatTransmissionBase *flatTransmission = check_and_cast<const FlatTransmissionBase *>(transmission);
    const Ieee80211TransmissionBase *ieee80211Transmission = check_and_cast<const Ieee80211TransmissionBase *>(transmission);
    const IIeee80211Mode *mode = ieee80211Transmission->getMode();
    // probability of no bit error in the header
    double minSNIR = snir->getMin();
    int headerBitLength = flatTransmission->getHeaderBitLength();
    double headerSuccessRate = GetChunkSuccessRate(mode->getHeaderMode(), minSNIR, headerBitLength);
    // probability of no bit error in the MPDU
    int payloadBitLength = flatTransmission->getPayloadBitLength();
    double payloadSuccessRate = GetChunkSuccessRate(mode->getDataMode(), minSNIR, payloadBitLength);
    EV_DEBUG << "min SNIR = " << minSNIR << ", bit length = " << payloadBitLength << ", header error rate = " << 1 - headerSuccessRate << ", payload error rate = " << 1 - payloadSuccessRate << endl;
    if (headerSuccessRate >= 1)
        headerSuccessRate = 1;
    if (payloadSuccessRate >= 1)
        payloadSuccessRate = 1;
    return 1 - headerSuccessRate * payloadSuccessRate;
}

double Ieee80211ErrorModelBase::computeBitErrorRate(const ISNIR *snir) const
{
    return NaN;
}

double Ieee80211ErrorModelBase::computeSymbolErrorRate(const ISNIR *snir) const
{
    return NaN;
}

} // namespace physicallayer

} // namespace inet

