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

#include "inet/physicallayer/apskradio/packetlevel/APSKScalarTransmission.h"
#include "inet/physicallayer/apskradio/packetlevel/APSKScalarReceiver.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarReception.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarSNIR.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKScalarReceiver);

APSKScalarReceiver::APSKScalarReceiver() :
    FlatReceiverBase()
{
}

std::ostream& APSKScalarReceiver::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKScalarReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

bool APSKScalarReceiver::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    auto apskTransmission = dynamic_cast<const APSKScalarTransmission *>(transmission);
    return apskTransmission && NarrowbandReceiverBase::computeIsReceptionPossible(listening, transmission);
}

bool APSKScalarReceiver::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    auto apksTransmission = dynamic_cast<const APSKScalarTransmission *>(reception->getTransmission());
    return apksTransmission && FlatReceiverBase::computeIsReceptionPossible(listening, reception, part);
}

} // namespace physicallayer

} // namespace inet

