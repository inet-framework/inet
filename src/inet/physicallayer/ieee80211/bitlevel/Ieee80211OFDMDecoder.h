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

#ifndef __INET_IEEE80211OFDMDECODER_H
#define __INET_IEEE80211OFDMDECODER_H

#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaver.h"
#include "inet/physicallayer/common/layered/AdditiveScrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Interleaving.h"
#include "inet/physicallayer/common/layered/ConvolutionalCoder.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMCode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/base/APSKModulationBase.h"
#include "inet/physicallayer/contract/layered/ISignalPacketModel.h"
#include "inet/physicallayer/contract/layered/ISignalBitModel.h"
#include "inet/physicallayer/contract/layered/IDecoder.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMDecoder : public IDecoder
{
  protected:
    const Ieee80211OFDMCode *code;
    const IScrambler *descrambler;
    const IFECCoder *fecDecoder;
    const IInterleaver *deinterleaver;

  protected:
    const IReceptionPacketModel *createPacketModel(const BitVector *decodedBits, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const;
    ShortBitVector getSignalFieldRate(const BitVector& signalField) const;
    unsigned int getSignalFieldLength(const BitVector& signalField) const;
    unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const;

  public:
    Ieee80211OFDMDecoder(const IScrambler *descrambler, const IFECCoder *fecDecoder, const IInterleaver *deinterleaver);
    Ieee80211OFDMDecoder(const Ieee80211OFDMCode *code);
    virtual ~Ieee80211OFDMDecoder();

    virtual void printToStream(std::ostream& stream) const { stream << "IEEE 802.11 OFDM Decoder"; }
    const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
    const Ieee80211OFDMCode *getCode() const { return code; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMDECODER_H

