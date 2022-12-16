//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDecoder.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IApskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDefs.h"

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

std::ostream& Ieee80211OfdmDecoder::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmDecoder";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(code, printFieldToString(code, level + 1, evFlags))
               << EV_FIELD(descrambler, printFieldToString(descrambler, level + 1, evFlags))
               << EV_FIELD(fecDecoder, printFieldToString(fecDecoder, level + 1, evFlags))
               << EV_FIELD(deinterleaver, printFieldToString(deinterleaver, level + 1, evFlags));
    return stream;
}

const IReceptionPacketModel *Ieee80211OfdmDecoder::decode(const IReceptionBitModel *bitModel) const
{
    bool hasBitError = false;
    BitVector *decodedBits = new BitVector(*bitModel->getAllBits());
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
    return new ReceptionPacketModel(packet, bps(NaN), bps(NaN));
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

