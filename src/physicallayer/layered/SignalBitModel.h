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
#include "BitVector.h"
#include "ISignalBitModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalBitModel : public virtual ISignalBitModel
{
  protected:
    const int bitLength;
    const double bitRate;
    const BitVector& bits;
    const ICyclicRedundancyCheck *cyclicRedundancyCheck;
    const IForwardErrorCorrection *forwardErrorCorrection;

  public:
    SignalBitModel() :
        bitLength(-1),
        bitRate(sNaN),
        bits(BitVector::UNDEF),
        cyclicRedundancyCheck(NULL),
        forwardErrorCorrection(NULL)
    {}

    SignalBitModel(int bitLength, double bitRate, const BitVector& bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection) :
        bitLength(bitLength),
        bitRate(bitRate),
        bits(bits),
        cyclicRedundancyCheck(cyclicRedundancyCheck),
        forwardErrorCorrection(forwardErrorCorrection)
    {}

    virtual void printToStream(std::ostream &stream) const;

    virtual int getBitLength() const { return bitLength; }

    virtual double getBitRate() const { return bitRate; }

    virtual const BitVector& getBits() const { return bits; }

    virtual const ICyclicRedundancyCheck *getCyclicRedundancyCheck() const { return cyclicRedundancyCheck; }

    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
};

class INET_API TransmissionBitModel : public SignalBitModel, public virtual ITransmissionBitModel
{
  public:
    TransmissionBitModel() :
        SignalBitModel()
    {}

    TransmissionBitModel(int bitLength, double bitRate, const BitVector& bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection) :
        SignalBitModel(bitLength, bitRate, bits, cyclicRedundancyCheck, forwardErrorCorrection)
    {}
};

class INET_API ReceptionBitModel : public SignalBitModel, public virtual IReceptionBitModel
{
  protected:
    const double ber;
    const int bitErrorCount;

  public:
    ReceptionBitModel() :
        SignalBitModel(),
        ber(sNaN),
        bitErrorCount(-1)
    {}

    ReceptionBitModel(int bitLength, double bitRate, const BitVector& bits, const ICyclicRedundancyCheck *cyclicRedundancyCheck, const IForwardErrorCorrection *forwardErrorCorrection, double ber, int bitErrorCount) :
        SignalBitModel(bitLength, bitRate, bits, cyclicRedundancyCheck, forwardErrorCorrection),
        ber(ber),
        bitErrorCount(bitErrorCount)
    {}

    virtual double getBER() const { return ber; }

    virtual int getBitErrorCount() const { return bitErrorCount; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SIGNALBITMODEL_H
