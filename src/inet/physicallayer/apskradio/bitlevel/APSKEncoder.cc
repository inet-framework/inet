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

#include "inet/physicallayer/apskradio/bitlevel/APSKEncoder.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKPhyFrameSerializer.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKEncoder);

APSKEncoder::APSKEncoder() :
    code(nullptr),
    scrambler(nullptr),
    fecEncoder(nullptr),
    interleaver(nullptr)
{
}

APSKEncoder::~APSKEncoder()
{
    delete code;
}

void APSKEncoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        scrambler = dynamic_cast<IScrambler *>(getSubmodule("scrambler"));
        fecEncoder = dynamic_cast<IFECCoder *>(getSubmodule("fecEncoder"));
        interleaver = dynamic_cast<IInterleaver *>(getSubmodule("interleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const IScrambling *scrambling = scrambler != nullptr ? scrambler->getScrambling() : nullptr;
        const ConvolutionalCode *forwardErrorCorrection = fecEncoder != nullptr ? check_and_cast<const ConvolutionalCode *>(fecEncoder->getForwardErrorCorrection()) : nullptr;
        const IInterleaving *interleaving = interleaver != nullptr ? interleaver->getInterleaving() : nullptr;
        code = new APSKCode(forwardErrorCorrection, interleaving, scrambling);
    }
}

std::ostream& APSKEncoder::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKEncoder";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", code = " << printObjectToString(code, level - 1);
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", scrambler = " << printObjectToString(scrambler, level - 1)
               << ", fecEncoder = " << printObjectToString(fecEncoder, level - 1)
               << ", interleaver = " << printObjectToString(interleaver, level - 1);
    return stream;
}

const ITransmissionBitModel *APSKEncoder::encode(const ITransmissionPacketModel *packetModel) const
{
    const BitVector *serializedBits = packetModel->getSerializedPacket();
    BitVector *encodedBits = new BitVector(*serializedBits);
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
    int netHeaderBitLength = APSK_PHY_FRAME_HEADER_BYTE_LENGTH * 8;
    if (forwardErrorCorrection == nullptr)
        return new TransmissionBitModel(netHeaderBitLength, packetModel->getBitrate(), serializedBits->getSize() - netHeaderBitLength, packetModel->getBitrate(), encodedBits, forwardErrorCorrection, scrambling, interleaving);
    else {
        int grossHeaderBitLength = forwardErrorCorrection->getEncodedLength(netHeaderBitLength);
        bps grossBitrate = packetModel->getBitrate() / forwardErrorCorrection->getCodeRate();
        return new TransmissionBitModel(grossHeaderBitLength, grossBitrate, serializedBits->getSize() - grossHeaderBitLength, grossBitrate, encodedBits, forwardErrorCorrection, scrambling, interleaving);
    }
}

} // namespace physicallayer

} // namespace inet

