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

#include "DimensionalTransmitter.h"
#include "DimensionalTransmission.h"
#include "DimensionalUtils.h"
#include "IRadio.h"
#include "IMobility.h"

namespace inet {

namespace physicallayer {

Define_Module(DimensionalTransmitter);

void DimensionalTransmitter::initialize(int stage)
{
    FlatTransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        const char *dimensionsString = par("dimensions");
        // TODO: move parsing?
        cStringTokenizer tokenizer(dimensionsString);
        while (tokenizer.hasMoreTokens()) {
            const char *dimensionString = tokenizer.nextToken();
            if (!strcmp("time", dimensionString))
                dimensions.addDimension(Dimension::time);
            else if (!strcmp("frequency", dimensionString))
                dimensions.addDimension(Dimension::frequency);
            else
                throw cRuntimeError("Unknown dimension");
        }
    }
}

void DimensionalTransmitter::printToStream(std::ostream& stream) const
{
    stream << "dimensional transmitter, "
           << "bitrate = " << bitrate << ", "
           << "power = " << power << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const ITransmission *DimensionalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    const simtime_t duration = macFrame->getBitLength() / bitrate.get();
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = DimensionalUtils::createFlatMapping(dimensions, startTime, endTime, carrierFrequency, bandwidth, power);
    return new DimensionalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, bitrate, powerMapping);
}

} // namespace physicallayer

} // namespace inet

