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

#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/contract/packetlevel/IApskModulation.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDecoder.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/modulation/BpskModulation.h"
#include "inet/physicallayer/modulation/Qam16Modulation.h"
#include "inet/physicallayer/modulation/Qam64Modulation.h"
#include "inet/physicallayer/modulation/QpskModulation.h"

namespace inet {
namespace physicallayer {

Ieee80211OfdmDecoder::Ieee80211OfdmDecoder(const Ieee80211OfdmCode *code) :
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
        deinterleaver = new Ieee80211OfdmInterleaver(code->getInterleaving());
}

std::ostream& Ieee80211OfdmDecoder::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OfdmDecoder";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", code = " << printObjectToString(code, level + 1)
               << ", descrambler = " << printObjectToString(descrambler, level + 1)
               << ", fecDecoder = " << printObjectToString(fecDecoder, level + 1)
               << ", deinterleaver = " << printObjectToString(deinterleaver, level + 1);
    return stream;
}

const IReceptionPacketModel *Ieee80211OfdmDecoder::decode(const IReceptionBitModel *bitModel) const
{
    bool hasBitError = false;
    BitVector *decodedBits = new BitVector(*bitModel->getBits());
    const IInterleaving *interleaving = nullptr;
    if (deinterleaver) {
        *decodedBits = deinterleaver->deinterleave(*decodedBits);
        interleaving = deinterleaver->getInterleaving();
    }
    const IForwardErrorCorrection *forwardErrorCorrection = nullptr;
    if (fecDecoder) {
        std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(*decodedBits);
        hasBitError = !fecDecodedDataField.second;
        *decodedBits = fecDecodedDataField.first;
        forwardErrorCorrection = fecDecoder->getForwardErrorCorrection();
    }
    const IScrambling *scrambling = nullptr;
    if (descrambler) {
        scrambling = descrambler->getScrambling();
        *decodedBits = descrambler->descramble(*decodedBits);
    }
    return createPacketModel(decodedBits, hasBitError, scrambling, forwardErrorCorrection, interleaving);
}

const IReceptionPacketModel *Ieee80211OfdmDecoder::createPacketModel(const BitVector *decodedBits, bool hasBitError, const IScrambling *scrambling, const IForwardErrorCorrection *fec, const IInterleaving *interleaving) const
{
    Packet *packet;
    if (decodedBits->getSize() % 8 == 0) {
        const auto& bytesChunk = makeShared<BytesChunk>(decodedBits->getBytes());
        packet = new Packet(nullptr, bytesChunk);
    }
    else {
        std::vector<bool> bits;
        for (int i = 0; i < (int)decodedBits->getSize(); i++)
            bits.push_back(decodedBits->getBit(i));
        const auto& bitsChunk = makeShared<BitsChunk>(bits);
        packet = new Packet(nullptr, bitsChunk);
    }
    delete decodedBits;
    packet->setBitError(hasBitError);
    return new ReceptionPacketModel(packet, bps(NaN));
}

ShortBitVector Ieee80211OfdmDecoder::getSignalFieldRate(const BitVector& signalField) const
{
    ShortBitVector rate;
    for (int i = SIGNAL_RATE_FIELD_START; i <= SIGNAL_RATE_FIELD_END; i++)
        rate.appendBit(signalField.getBit(i));
    return rate;
}

unsigned int Ieee80211OfdmDecoder::getSignalFieldLength(const BitVector& signalField) const
{
    ShortBitVector length;
    for (int i = SIGNAL_LENGTH_FIELD_START; i <= SIGNAL_LENGTH_FIELD_END; i++)
        length.appendBit(signalField.getBit(i));
    return length.toDecimal();
}

unsigned int Ieee80211OfdmDecoder::calculatePadding(unsigned int dataFieldLengthInBits, const IModulation *modulationScheme, const Ieee80211ConvolutionalCode *fec) const
{
    const IApskModulation *dataModulationScheme = dynamic_cast<const IApskModulation *>(modulationScheme);
    ASSERT(dataModulationScheme != nullptr);
    unsigned int codedBitsPerOFDMSymbol = dataModulationScheme->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol * fec->getCodeRatePuncturingK() / fec->getCodeRatePuncturingN();
    return dataBitsPerOFDMSymbol - dataFieldLengthInBits % dataBitsPerOFDMSymbol;
}

Ieee80211OfdmDecoder::~Ieee80211OfdmDecoder()
{
    delete deinterleaver;
    delete descrambler;
    delete fecDecoder;
}

} /* namespace physicallayer */
} /* namespace inet */

