//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/physicallayer/common/layered/LayeredTransmission.h"

namespace inet {

namespace physicallayer {

LayeredTransmission::LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation) :
    TransmissionBase(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation),
    packetModel(packetModel),
    bitModel(bitModel),
    symbolModel(symbolModel),
    sampleModel(sampleModel),
    analogModel(analogModel)
{
}

LayeredTransmission::~LayeredTransmission()
{
    delete packetModel;
    delete bitModel;
    delete symbolModel;
    delete sampleModel;
    delete analogModel;
}

} // namespace physicallayer

} // namespace inet

