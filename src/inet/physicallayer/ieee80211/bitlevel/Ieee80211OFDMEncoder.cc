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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMEncoder.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMInterleaver.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/common/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/common/bitlevel/ConvolutionalCoder.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDefs.h"

namespace inet {

namespace physicallayer {

std::ostream& Ieee80211OFDMEncoder::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211OFDMEncoder";
    if (level >= PRINT_LEVEL_TRACE)
        stream << ", convolutionalCoder = " << printObjectToString(convolutionalCoder, level - 1)
               << ", interleaver = " << printObjectToString(interleaver, level - 1)
               << ", scrambler = " << printObjectToString(scrambler, level - 1)
               << ", code = " << printObjectToString(code, level - 1);
    return stream;
}

const ITransmissionBitModel *Ieee80211OFDMEncoder::encode(const ITransmissionPacketModel *packetModel) const
{
    const BitVector *serializedPacket = packetModel->getSerializedPacket();
    BitVector *encodedBits = new BitVector(*serializedPacket);
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

Ieee80211OFDMEncoder::Ieee80211OFDMEncoder(const Ieee80211OFDMCode *code) :
    convolutionalCoder(nullptr),
    interleaver(nullptr),
    scrambler(nullptr),
    code(code)
{
    if (code->getScrambling())
        scrambler = new AdditiveScrambler(code->getScrambling());
    if (code->getInterleaving())
        interleaver = new Ieee80211OFDMInterleaver(code->getInterleaving());
    if (code->getConvolutionalCode())
        convolutionalCoder = new ConvolutionalCoder(code->getConvolutionalCode());
}

Ieee80211OFDMEncoder::~Ieee80211OFDMEncoder()
{
    delete convolutionalCoder;
    delete interleaver;
    delete scrambler;
}
} /* namespace physicallayer */
} /* namespace inet */

