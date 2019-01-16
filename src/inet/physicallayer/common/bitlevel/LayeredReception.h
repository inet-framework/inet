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

#ifndef __INET_LAYEREDRECEPTION_H
#define __INET_LAYEREDRECEPTION_H

#include "inet/physicallayer/base/packetlevel/ReceptionBase.h"
#include "inet/physicallayer/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredReception : public ReceptionBase
{
  protected:
//     TODO: where are the other models?
//     const IReceptionPacketModel *packetModel;
//     const IReceptionBitModel    *bitModel;
//     const IReceptionSymbolModel *symbolModel;
//     const IReceptionSampleModel *sampleModel;
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredReception(const IReceptionAnalogModel *analogModel, const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation);
    virtual ~LayeredReception();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReceptionAnalogModel *getAnalogModel() const override { return analogModel; }
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_LAYEREDRECEPTION_H

