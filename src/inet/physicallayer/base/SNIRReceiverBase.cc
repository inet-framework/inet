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

#include "inet/physicallayer/base/SNIRReceiverBase.h"
#include "inet/physicallayer/common/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

SNIRReceiverBase::SNIRReceiverBase() :
    ReceiverBase(),
    snirThreshold(sNaN)
{
}

void SNIRReceiverBase::initialize(int stage)
{
    ReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        snirThreshold = math::dB2fraction(par("snirThreshold"));
}

void SNIRReceiverBase::printToStream(std::ostream& stream) const
{
    stream << "snirThreshold = " << snirThreshold;
}

bool SNIRReceiverBase::areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const
{
    return carrierFrequency1 + bandwidth1 / 2 >= carrierFrequency2 - bandwidth2 / 2 &&
           carrierFrequency1 - bandwidth1 / 2 <= carrierFrequency2 + bandwidth2 / 2;
}

const RadioReceptionIndication *SNIRReceiverBase::computeReceptionIndication(const ISNIR *snir) const
{
    RadioReceptionIndication *indication = new RadioReceptionIndication();
    indication->setMinSNIR(snir->getMin());
    return indication;
}

bool SNIRReceiverBase::computeIsReceptionSuccessful(const ISNIR *snir) const
{
    return snir->getMin() > snirThreshold;
}

const IReceptionDecision *SNIRReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, const IInterference *interference) const
{
    const ISNIR *snir = computeSNIR(reception, computeNoise(listening, interference));
    bool isReceptionPossible = computeIsReceptionPossible(listening, reception);
    bool isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, interference);
    bool isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(snir);
    const RadioReceptionIndication *indication = isReceptionAttempted ? computeReceptionIndication(snir) : NULL;
    return new ReceptionDecision(reception, indication, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

} // namespace physicallayer

} // namespace inet

