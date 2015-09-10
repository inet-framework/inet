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

#ifndef __INET_LAYEREDRECEPTIONDECISION_H
#define __INET_LAYEREDRECEPTIONDECISION_H

#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API LayeredReceptionDecision : public ReceptionDecision
{
  protected:
    const IReceptionPacketModel *packetModel;
    const IReceptionBitModel *bitModel;
    const IReceptionSymbolModel *symbolModel;
    const IReceptionSampleModel *sampleModel;
    const IReceptionAnalogModel *analogModel;

  public:
    LayeredReceptionDecision(const IReception *reception, const ReceptionIndication *indication, const IReceptionPacketModel *packetModel, const IReceptionBitModel *bitModel, const IReceptionSymbolModel *symbolModel, const IReceptionSampleModel *sampleModel, const IReceptionAnalogModel *analogModel, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful);
    virtual ~LayeredReceptionDecision();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IReceptionPacketModel *getPacketModel() const { return packetModel; }
    virtual const IReceptionBitModel *getBitModel() const { return bitModel; }
    virtual const IReceptionSymbolModel *getSymbolModel() const { return symbolModel; }
    virtual const IReceptionSampleModel *getSampleModel() const { return sampleModel; }
    virtual const IReceptionAnalogModel *getAnalogModel() const { return analogModel; }

    virtual const cPacket *getPhyFrame() const override;
    virtual const cPacket *getMacFrame() const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_LAYEREDRECEPTIONDECISION_H

