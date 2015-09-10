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

#ifndef __INET_SIGNALBITMODEL_H
#define __INET_SIGNALBITMODEL_H

#include <vector>
#include "inet/common/BitVector.h"
#include "inet/physicallayer/contract/bitlevel/ISignalBitModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalBitModel : public virtual ISignalBitModel
{
  protected:
    const BitVector *bits;
    const int headerBitLength;
    const bps headerBitRate;
    const int payloadBitLength;
    const bps payloadBitRate;

  public:
    SignalBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits);
    virtual ~SignalBitModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual int getHeaderBitLength() const override { return headerBitLength; }
    virtual bps getHeaderBitRate() const override { return headerBitRate; }
    virtual int getPayloadBitLength() const override { return payloadBitLength; }
    virtual bps getPayloadBitRate() const override { return payloadBitRate; }
    virtual const BitVector *getBits() const override { return bits; }
};

class INET_API TransmissionBitModel : public SignalBitModel, public virtual ITransmissionBitModel
{
  protected:
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IScrambling *scrambling;
    const IInterleaving *interleaving;

  public:
    TransmissionBitModel(const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving);
    TransmissionBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving);

    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    virtual const IScrambling *getScrambling() const { return scrambling; }
    virtual const IInterleaving *getInterleaving() const { return interleaving; }
};

class INET_API ReceptionBitModel : public SignalBitModel, public virtual IReceptionBitModel
{
  public:
    ReceptionBitModel(int headerBitLength, bps headerBitRate, int payloadBitLength, bps payloadBitRate, const BitVector *bits);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SIGNALBITMODEL_H

