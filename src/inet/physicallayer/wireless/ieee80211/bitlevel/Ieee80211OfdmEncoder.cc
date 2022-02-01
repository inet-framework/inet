//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoder.h"

#include "inet/common/ShortBitVector.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoder.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaver.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {

namespace physicallayer {

std::ostream& Ieee80211OfdmEncoder::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmEncoder";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(convolutionalCoder, printFieldToString(convolutionalCoder, level + 1, evFlags))
               << EV_FIELD(interleaver, printFieldToString(interleaver, level + 1, evFlags))
               << EV_FIELD(scrambler, printFieldToString(scrambler, level + 1, evFlags))
               << EV_FIELD(code, printFieldToString(code, level + 1, evFlags));
    return stream;
}

const ITransmissionBitModel *Ieee80211OfdmEncoder::encode(const ITransmissionPacketModel *packetModel) const
{
    auto packet = packetModel->getPacket();
    auto length = packet->getTotalLength();
    BitVector *encodedBits;
    if (b(length).get() % 8 == 0) {
        auto bytes = packet->peekAllAsBytes()->getBytes();
        encodedBits = new BitVector(bytes);
    }
    else {
        encodedBits = new BitVector();
        const auto& bitsChunk = packet->peekAllAsBits();
        for (int i = 0; i < b(length).get(); i++)
            encodedBits->appendBit(bitsChunk->getBit(i));
    }
    const IScrambling *scrambling = nullptr;
    if (scrambler) {
        *encodedBits = scrambler->scramble(*encodedBits);
        scrambling = scrambler->getScrambling();
        EV_DEBUG << "Scrambled bits are: " << *encodedBits << endl;
    }
    const IForwardErrorCorrection *forwardErrorCorrection = nullptr;
    if (convolutionalCoder) {
        *encodedBits = convolutionalCoder->encode(*encodedBits);
        forwardErrorCorrection = convolutionalCoder->getForwardErrorCorrection();
        EV_DEBUG << "FEC encoded bits are: " << *encodedBits << endl;
    }
    const IInterleaving *interleaving = nullptr;
    if (interleaver) {
        *encodedBits = interleaver->interleave(*encodedBits);
        interleaving = interleaver->getInterleaving();
        EV_DEBUG << "Interleaved bits are: " << *encodedBits << endl;
    }
    return new TransmissionBitModel(encodedBits, forwardErrorCorrection, scrambling, interleaving);
}

Ieee80211OfdmEncoder::Ieee80211OfdmEncoder(const Ieee80211OfdmCode *code) :
    convolutionalCoder(nullptr),
    interleaver(nullptr),
    scrambler(nullptr),
    code(code)
{
    if (code->getScrambling())
        scrambler = new AdditiveScrambler(code->getScrambling());
    if (code->getInterleaving())
        interleaver = new Ieee80211OfdmInterleaver(code->getInterleaving());
    if (code->getConvolutionalCode())
        convolutionalCoder = new ConvolutionalCoder(code->getConvolutionalCode());
}

Ieee80211OfdmEncoder::~Ieee80211OfdmEncoder()
{
    delete convolutionalCoder;
    delete interleaver;
    delete scrambler;
}

} /* namespace physicallayer */
} /* namespace inet */

