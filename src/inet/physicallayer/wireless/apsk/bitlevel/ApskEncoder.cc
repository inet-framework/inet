//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/bitlevel/ApskEncoder.h"

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskEncoder);

ApskEncoder::ApskEncoder() :
    code(nullptr),
    scrambler(nullptr),
    fecEncoder(nullptr),
    interleaver(nullptr)
{
}

ApskEncoder::~ApskEncoder()
{
    delete code;
}

void ApskEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        fecEncoder = dynamic_cast<IFecCoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const IScrambling *scrambling = scrambler != nullptr ? scrambler->getScrambling() : nullptr;
        const ConvolutionalCode *forwardErrorCorrection = fecEncoder != nullptr ? check_and_cast<const ConvolutionalCode *>(fecEncoder->getForwardErrorCorrection()) : nullptr;
        const IInterleaving *interleaving = interleaver != nullptr ? interleaver->getInterleaving() : nullptr;
        code = new ApskCode(forwardErrorCorrection, interleaving, scrambling);
    }
}

std::ostream& ApskEncoder::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskEncoder";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(code, printFieldToString(code, level + 1, evFlags));
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(scrambler, printFieldToString(scrambler, level + 1, evFlags))
               << EV_FIELD(fecEncoder, printFieldToString(fecEncoder, level + 1, evFlags))
               << EV_FIELD(interleaver, printFieldToString(interleaver, level + 1, evFlags));
    return stream;
}

const ITransmissionBitModel *ApskEncoder::encode(const ITransmissionPacketModel *packetModel) const
{
    auto packet = packetModel->getPacket();
    const auto& apskPhyHeader = packet->peekAtFront<ApskPhyHeader>();
    auto length = packet->getTotalLength();
    BitVector *encodedBits;
    if (b(length).get() % 8 == 0)
        encodedBits = new BitVector(packet->peekAllAsBytes()->getBytes());
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
    if (fecEncoder) {
        *encodedBits = fecEncoder->encode(*encodedBits);
        forwardErrorCorrection = fecEncoder->getForwardErrorCorrection();
        EV_DEBUG << "FEC encoded bits are: " << *encodedBits << endl;
    }
    const IInterleaving *interleaving = nullptr;
    if (interleaver) {
        *encodedBits = interleaver->interleave(*encodedBits);
        interleaving = interleaver->getInterleaving();
        EV_DEBUG << "Interleaved bits are: " << *encodedBits << endl;
    }
    b netHeaderLength = apskPhyHeader->getChunkLength();
    if (forwardErrorCorrection == nullptr)
        return new TransmissionBitModel(netHeaderLength, packetModel->getDataNetBitrate(), length - netHeaderLength, packetModel->getDataNetBitrate(), encodedBits, forwardErrorCorrection, scrambling, interleaving);
    else {
        b grossHeaderLength = b(forwardErrorCorrection->getEncodedLength(b(netHeaderLength).get()));
        bps grossBitrate = packetModel->getDataNetBitrate() / forwardErrorCorrection->getCodeRate();
        return new TransmissionBitModel(grossHeaderLength, grossBitrate, length - grossHeaderLength, grossBitrate, encodedBits, forwardErrorCorrection, scrambling, interleaving);
    }
}

} // namespace physicallayer

} // namespace inet

