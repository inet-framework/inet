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
#include "inet/physicallayer/contract/ISignalBitModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalBitModel : public virtual ISignalBitModel
{
  protected:
    const BitVector *bits;
    const int bitLength;
    const double bitRate;

  public:
    SignalBitModel() :
        bits(&BitVector::UNDEF),
        bitLength(-1),
        bitRate(sNaN)
    {}

    SignalBitModel(int bitLength, double bitRate, const BitVector *bits) :
        bits(bits),
        bitLength(bitLength),
        bitRate(bitRate)
    {}

    virtual void printToStream(std::ostream &stream) const;
    virtual int getBitLength() const { return bitLength; }
    virtual double getBitRate() const { return bitRate; }
    virtual const BitVector* getBits() const { return bits; }
};

class INET_API TransmissionBitModel : public SignalBitModel, public virtual ITransmissionBitModel
{
  protected:
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IScrambling *scrambling;
    const IInterleaving *interleaving;

  public:
    TransmissionBitModel() :
        SignalBitModel()
    {}

    TransmissionBitModel(int bitLength, double bitRate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scramblerInfo, const IInterleaving *interleaverInfo) :
        SignalBitModel(bitLength, bitRate, bits),
        forwardErrorCorrection(forwardErrorCorrection),
        scrambling(scramblerInfo),
        interleaving(interleaverInfo)
    {}

    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
    virtual const IScrambling *getScrambling() const { return scrambling; }
    virtual const IInterleaving *getInterleaving() const { return interleaving; }
};

class INET_API ReceptionBitModel : public SignalBitModel, public virtual IReceptionBitModel
{
  protected:
    const IModulation *modulation;
    const double ber;
    const int bitErrorCount;

  public:
    ReceptionBitModel() :
        SignalBitModel(),
        modulation(NULL),
        ber(sNaN),
        bitErrorCount(-1)
    {}

    ReceptionBitModel(int bitLength, double bitRate, const BitVector *bits, const IModulation *modulation, double ber, int bitErrorCount) :
        SignalBitModel(bitLength, bitRate, bits),
        modulation(modulation),
        ber(ber),
        bitErrorCount(bitErrorCount)
    {}

    virtual double getBER() const { return ber; }
    virtual int getBitErrorCount() const { return bitErrorCount; }
    const IModulation *getModulation() const { return modulation; }
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_SIGNALBITMODEL_H
