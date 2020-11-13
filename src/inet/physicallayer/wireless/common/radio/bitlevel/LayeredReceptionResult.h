//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_LAYEREDRECEPTIONRESULT_H
#define __INET_LAYEREDRECEPTIONRESULT_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionResult.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredReceptionResult : public ReceptionResult
{
  protected:
    const IReceptionPacketModel *packetModel;
    const IReceptionBitModel *bitModel;
    const IReceptionSymbolModel *symbolModel;
    const IReceptionSampleModel *sampleModel;
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const IReceptionPacketModel *packetModel, const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel, const IReceptionSampleModel *sampleModel, const IReceptionAnalogModel *analogModel);
    virtual ~LayeredReceptionResult();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IReceptionPacketModel *getPacketModel() const { return packetModel; }
    virtual const IReceptionBitModel *getBitModel() const { return bitModel; }
    virtual const IReceptionSymbolModel *getSymbolModel() const { return symbolModel; }
    virtual const IReceptionSampleModel *getSampleModel() const { return sampleModel; }
    virtual const IReceptionAnalogModel *getAnalogModel() const { return analogModel; }

    virtual const Packet *getPacket() const override;
};

} // namespace physicallayer
} // namespace inet

#endif

