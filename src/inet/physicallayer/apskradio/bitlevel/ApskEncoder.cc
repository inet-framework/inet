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

#include "inet/common/packet/chunk/BitsChunk.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskEncoder.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"

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

std::ostream& ApskEncoder::printToStream(std::ostream& stream, int level) const
{
    stream << "ApskEncoder";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", code = " << printObjectToString(code, level + 1);
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", scrambler = " << printObjectToString(scrambler, level + 1)
               << ", fecEncoder = " << printObjectToString(fecEncoder, level + 1)
               << ", interleaver = " << printObjectToString(interleaver, level + 1);
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
        return new TransmissionBitModel(netHeaderLength, packetModel->getBitrate(), length - netHeaderLength, packetModel->getBitrate(), encodedBits, forwardErrorCorrection, scrambling, interleaving);
    else {
        b grossHeaderLength = b(forwardErrorCorrection->getEncodedLength(b(netHeaderLength).get()));
        bps grossBitrate = packetModel->getBitrate() / forwardErrorCorrection->getCodeRate();
        return new TransmissionBitModel(grossHeaderLength, grossBitrate, length - grossHeaderLength, grossBitrate, encodedBits, forwardErrorCorrection, scrambling, interleaving);
    }
}

} // namespace physicallayer

} // namespace inet

