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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDecoder.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/contract/packetlevel/IAPSKModulation.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDefs.h"

namespace inet {

namespace physicallayer {

Ieee80211OFDMDecoder::Ieee80211OFDMDecoder(const Ieee80211OFDMCode *code) :
    code(code),
    descrambler(nullptr),
    fecDecoder(nullptr),
    deinterleaver(nullptr)
{
    if (code->getScrambling())
        descrambler = new AdditiveScrambler(code->getScrambling());
    if (code->getConvolutionalCode())
        fecDecoder = new ConvolutionalCoder(code->getConvolutionalCode());
    if (code->getInterleaving())
        deinterleaver = new Ieee80211OFDMInterleaver(code->getInterleaving());
}

std::ostream& Ieee80211OFDMDecoder::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OFDMDecoder";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", code = " << printObjectToString(code, level - 1)
               << ", descrambler = " << printObjectToString(descrambler, level - 1)
               << ", fecDecoder = " << printObjectToString(fecDecoder, level - 1)
               << ", deinterleaver = " << printObjectToString(deinterleaver, level - 1);
    return stream;
}

const IReceptionPacketModel *Ieee80211OFDMDecoder::decode(const IReceptionBitModel *bitModel) const
{
    BitVector *decodedBits = new BitVector(*bitModel->getBits());
    const IInterleaving *interleaving = nullptr;
    if (deinterleaver) {
        *decodedBits = deinterleaver->deinterleave(*decodedBits);
        interleaving = deinterleaver->getInterleaving();
    }
    const IForwardErrorCorrection *forwardErrorCorrection = nullptr;
    if (fecDecoder) {
        std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(*decodedBits);
        bool isDecodedSuccessfully = fecDecodedDataField.second;
        if (!isDecodedSuccessfully)
            throw cRuntimeError("FEC error");  // TODO: implement correct error handling
        *decodedBits = fecDecodedDataField.first;
        forwardErrorCorrection = fecDecoder->getForwardErrorCorrection();
    }
    const IScrambling *scrambling = nullptr;
    if (descrambler) {
        scrambling = descrambler->getScrambling();
        *decodedBits = descrambler->descramble(*decodedBits);
    }
    return createPacketModel(decodedBits, scrambling, forwardErrorCorrection, interleaving);
}

const IReceptionPacketModel *Ieee80211OFDMDecoder::createPacketModel(const BitVector *decodedBits, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const
{
    double per = -1;
    bool packetErrorless = true; // TODO: compute packet error rate, packetErrorLess
    return new ReceptionPacketModel(nullptr, decodedBits, bps(NaN), per, packetErrorless);
}

ShortBitVector Ieee80211OFDMDecoder::getSignalFieldRate(const BitVector& signalField) const
{
    ShortBitVector rate;
    for (int i = SIGNAL_RATE_FIELD_START; i <= SIGNAL_RATE_FIELD_END; i++)
        rate.appendBit(signalField.getBit(i));
    return rate;
}

unsigned int Ieee80211OFDMDecoder::getSignalFieldLength(const BitVector& signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField.getBit(i));
    return length.toDecimal();
}

unsigned int Ieee80211OFDMDecoder::calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const
{
    const IAPSKModulation *dataModulationScheme = dynamic_cast<const IAPSKModulation *>(modulationScheme);
    ASSERT(dataModulationScheme != nullptr);
    unsigned int codedBitsPerOFDMSymbol = dataModulationScheme->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

Ieee80211OFDMDecoder::~Ieee80211OFDMDecoder()
{
    delete deinterleaver;
    delete descrambler;
    delete fecDecoder;
}
} /* namespace physicallayer */
} /* namespace inet */

