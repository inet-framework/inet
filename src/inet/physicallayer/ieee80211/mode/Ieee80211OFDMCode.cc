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

#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMCode.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMDefs.h"

namespace inet {
namespace physicallayer {

Ieee80211OFDMCode::Ieee80211OFDMCode(const ConvolutionalCode* convolutionalCode, const Ieee80211OFDMInterleaving* interleaving, const AdditiveScrambling* scrambling) :
        convolutionalCode(convolutionalCode),
        interleaving(interleaving),
        scrambling(scrambling)
{
}

std::ostream& Ieee80211OFDMCode::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKCode";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", convolutionalCode = " << printObjectToString(convolutionalCode, level - 1)
               << ", interleaving = " << printObjectToString(interleaving, level - 1)
               << ", scrambling = " << printObjectToString(scrambling, level - 1);
    return stream;
}

// Convolutional codes
const Ieee80211ConvolutionalCode Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2(1,2);
const Ieee80211ConvolutionalCode Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3(2,3);
const Ieee80211ConvolutionalCode Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4(3,4);

// Interleavings
const Ieee80211OFDMInterleaving Ieee80211OFDMCompliantCodes::ofdmBPSKInterleaving(NUMBER_OF_OFDM_DATA_SUBCARRIERS, 1);
const Ieee80211OFDMInterleaving Ieee80211OFDMCompliantCodes::ofdmQPSKInterleaving(2 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 2);
const Ieee80211OFDMInterleaving Ieee80211OFDMCompliantCodes::ofdmQAM16Interleaving(4 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 4);
const Ieee80211OFDMInterleaving Ieee80211OFDMCompliantCodes::ofdmQAM64Interleaving(6 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 6);

// Scrambler
const AdditiveScrambling Ieee80211OFDMCompliantCodes::ofdmScrambling("1011101", "0001001");

// Codes
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantCodes::ofdmBPSKInterleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantCodes::ofdmBPSKInterleaving, nullptr);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC3_4BPSKInterleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OFDMCompliantCodes::ofdmBPSKInterleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC1_2QPSKInterleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantCodes::ofdmQPSKInterleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC3_4QPSKInterleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OFDMCompliantCodes::ofdmQPSKInterleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC1_2QAM16Interleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantCodes::ofdmQAM16Interleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM16Interleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OFDMCompliantCodes::ofdmQAM16Interleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC2_3QAM64Interleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, &Ieee80211OFDMCompliantCodes::ofdmQAM64Interleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);
const Ieee80211OFDMCode Ieee80211OFDMCompliantCodes::ofdmCC3_4QAM64Interleaving(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OFDMCompliantCodes::ofdmQAM64Interleaving, &Ieee80211OFDMCompliantCodes::ofdmScrambling);

} /* namespace physicallayer */
} /* namespace inet */

