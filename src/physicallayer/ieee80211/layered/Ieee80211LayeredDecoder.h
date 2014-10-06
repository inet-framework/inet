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

#ifndef __INET_IEEE80211LAYEREDDECODER_H_
#define __INET_IEEE80211LAYEREDDECODER_H_

#include "LayeredDecoder.h"
#include "ISerializer.h"
#include "Ieee80211Interleaver.h"
#include "Ieee80211Scrambler.h"
#include "Ieee80211Interleaving.h"
#include "ConvolutionalCoder.h"
#include "Ieee80211ConvolutionalCode.h"
#include "APSKModulationBase.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211LayeredDecoder : public LayeredDecoder
{
    protected:
        const Ieee80211Scrambling *descrambling;
        const Ieee80211Scrambler *descrambler;
        const ConvolutionalCoder *signalFECDecoder;
        const Ieee80211Interleaver *signalDeinterleaver;
        const ISerializer *deserializer;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { cRuntimeError("This module doesn't handle self messages."); }
        BitVector decodeSignalField(const BitVector& signalField) const;
        BitVector decodeDataField(const BitVector& dataField, const ConvolutionalCoder& fecDecoder, const Ieee80211Interleaver& deinterleaver) const;
        const IReceptionPacketModel *createPacketModel(const BitVector& decodedBits, const Ieee80211Scrambling *scrambling, const Ieee80211ConvolutionalCode *fec, const Ieee80211Interleaving *interleaving) const;
        const Ieee80211ConvolutionalCode *getFecFromSignalFieldRate(const ShortBitVector& rate) const;
        const APSKModulationBase *getModulationFromSignalFieldRate(const ShortBitVector& rate) const;
        const Ieee80211Interleaving *getInterleavingFromModulation(const IModulation *modulationScheme) const;
        ShortBitVector getSignalFieldRate(const BitVector& signalField) const;
        unsigned int getSignalFieldLength(const BitVector& signalField) const;
        unsigned int calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const;

    public:
        const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        virtual ~Ieee80211LayeredDecoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211LAYEREDDECODER_H_ */
