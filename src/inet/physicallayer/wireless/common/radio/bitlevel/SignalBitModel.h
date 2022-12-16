//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALBITMODEL_H
#define __INET_SIGNALBITMODEL_H

#include <vector>

#include "inet/common/BitVector.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"

namespace inet {
namespace physicallayer {

class INET_API SignalBitModel : public virtual ISignalBitModel
{
  protected:
    const BitVector *bits;
    const b headerLength;
    const bps headerBitrate;
    const b dataLength;
    const bps dataBitrate;

  public:
    SignalBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits);
    virtual ~SignalBitModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual b getHeaderLength() const override { return headerLength; }
    virtual bps getHeaderGrossBitrate() const override { return headerBitrate; }
    virtual b getDataLength() const override { return dataLength; }
    virtual bps getDataGrossBitrate() const override { return dataBitrate; }
    virtual const BitVector *getAllBits() const override { return bits; }
};

class INET_API TransmissionBitModel : public SignalBitModel, public virtual ITransmissionBitModel
{
  protected:
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IScrambling *scrambling;
    const IInterleaving *interleaving;

  public:
    TransmissionBitModel(const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving);
    TransmissionBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits, const IForwardErrorCorrection *forwardErrorCorrection, const IScrambling *scrambling, const IInterleaving *interleaving);

    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const override { return forwardErrorCorrection; }
    virtual const IScrambling *getScrambling() const override { return scrambling; }
    virtual const IInterleaving *getInterleaving() const override { return interleaving; }
};

class INET_API ReceptionBitModel : public SignalBitModel, public virtual IReceptionBitModel
{
  protected:
    double bitErrorRate;

  public:
    ReceptionBitModel(b headerLength, bps headerBitrate, b dataLength, bps dataBitrate, const BitVector *bits, double bitErrorrate);

    virtual double getBitErrorRate() const override { return bitErrorRate; }
};

} // namespace physicallayer
} // namespace inet

#endif

