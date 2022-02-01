//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMDECODER_H
#define __INET_IEEE80211OFDMDECODER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IDecoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoder.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaver.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaving.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmCode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmDecoder : public IDecoder
{
  protected:
    const Ieee80211OfdmCode *code = nullptr;
    const IScrambler *descrambler = nullptr;
    const IFecCoder *fecDecoder = nullptr;
    const IInterleaver *deinterleaver = nullptr;

  protected:
    const IReceptionPacketModel *createPacketModel(const BitVector *decodedBits, bool hasBitError, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const;
    ShortBitVector getSignalFieldRate(const BitVector& signalField) const;
    unsigned int getSignalFieldLength(const BitVector& signalField) const;
    unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const;

  public:
    Ieee80211OfdmDecoder(const IScrambler *descrambler, const IFecCoder *fecDecoder, const IInterleaver *deinterleaver);
    Ieee80211OfdmDecoder(const Ieee80211OfdmCode *code);
    virtual ~Ieee80211OfdmDecoder();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const override;
    const Ieee80211OfdmCode *getCode() const { return code; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

