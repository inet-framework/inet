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
#include "inet/physicallayer/apskradio/bitlevel/ApskDecoder.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskDecoder);

ApskDecoder::ApskDecoder() :
    code(nullptr),
    descrambler(nullptr),
    fecDecoder(nullptr),
    deinterleaver(nullptr)
{
}

ApskDecoder::~ApskDecoder()
{
    delete code;
}

void ApskDecoder::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        descrambler = dynamic_cast<const IScrambler *>(getSubmodule("descrambler"));
        fecDecoder = dynamic_cast<const IFecCoder *>(getSubmodule("fecDecoder"));
        deinterleaver = dynamic_cast<const IInterleaver *>(getSubmodule("deinterleaver"));
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        const IScrambling *scrambling = descrambler != nullptr ? descrambler->getScrambling() : nullptr;
        const ConvolutionalCode *forwardErrorCorrection = fecDecoder != nullptr ? check_and_cast<const ConvolutionalCode *>(fecDecoder->getForwardErrorCorrection()) : nullptr;
        const IInterleaving *interleaving = deinterleaver != nullptr ? deinterleaver->getInterleaving() : nullptr;
        code = new ApskCode(forwardErrorCorrection, interleaving, scrambling);
    }
}

std::ostream& ApskDecoder::printToStream(std::ostream& stream, int level) const
{
    stream << "ApskDecoder";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", code = " << code;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", descrambler = " << printObjectToString(descrambler, level + 1)
               << ", fecDecoder = " << printObjectToString(fecDecoder, level + 1)
               << ", deinterleaver = " << printObjectToString(deinterleaver, level + 1);
    return stream;
}

const IReceptionPacketModel *ApskDecoder::decode(const IReceptionBitModel *bitModel) const
{
    bool hasBitError = false;
    BitVector *decodedBits = new BitVector(*bitModel->getBits());
    if (deinterleaver)
        *decodedBits = deinterleaver->deinterleave(*decodedBits);
    if (fecDecoder) {
        std::pair<BitVector, bool> fecDecodedDataField = fecDecoder->decode(*decodedBits);
        hasBitError = !fecDecodedDataField.second;
        *decodedBits = fecDecodedDataField.first;
    }
    if (descrambler)
        *decodedBits = descrambler->descramble(*decodedBits);
    Packet *packet;
    if (decodedBits->getSize() % 8 == 0) {
        const auto& bytesChunk = makeShared<BytesChunk>(decodedBits->getBytes());
        packet = new Packet(nullptr, bytesChunk);
    }
    else {
        std::vector<bool> bits;
        for (uint32 i = 0; i < decodedBits->getSize(); i++)
            bits.push_back(decodedBits->getBit(i));
        const auto& bitsChunk = makeShared<BitsChunk>(bits);
        packet = new Packet(nullptr, bitsChunk);
    }
    delete decodedBits;
    packet->setBitError(hasBitError);
    return new ReceptionPacketModel(packet, bps(NaN));
}

} // namespace physicallayer

} // namespace inet

