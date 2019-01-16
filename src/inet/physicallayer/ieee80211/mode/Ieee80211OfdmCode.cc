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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmDefs.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmCode.h"
#include "inet/physicallayer/modulation/BpskModulation.h"

namespace inet {
namespace physicallayer {

Ieee80211OfdmCode::Ieee80211OfdmCode(const ConvolutionalCode* convolutionalCode, const Ieee80211OfdmInterleaving* interleaving, const AdditiveScrambling* scrambling) :
        convolutionalCode(convolutionalCode),
        interleaving(interleaving),
        scrambling(scrambling)
{
}

std::ostream& Ieee80211OfdmCode::printToStream(std::ostream& stream, int level) const
{
    stream << "ApskCode";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", convolutionalCode = " << printObjectToString(convolutionalCode, level + 1)
               << ", interleaving = " << printObjectToString(interleaving, level + 1)
               << ", scrambling = " << printObjectToString(scrambling, level + 1);
    return stream;
}

// Convolutional codes
const Ieee80211ConvolutionalCode Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2(1,2);
const Ieee80211ConvolutionalCode Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3(2,3);
const Ieee80211ConvolutionalCode Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4(3,4);
const Ieee80211ConvolutionalCode Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6(5,6);

// Interleavings
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmBPSKInterleaving(NUMBER_OF_OFDM_DATA_SUBCARRIERS, 1);
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmQPSKInterleaving(2 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 2);
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmQAM16Interleaving(4 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 4);
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmQAM64Interleaving(6 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 6);
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmQAM256Interleaving(8 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 8);
const Ieee80211OfdmInterleaving Ieee80211OfdmCompliantCodes::ofdmQAM1024Interleaving(10 * NUMBER_OF_OFDM_DATA_SUBCARRIERS, 10);

// Scrambler
const AdditiveScrambling Ieee80211OfdmCompliantCodes::ofdmScrambling("1011101", "0001001");

// Codes
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantCodes::ofdmBPSKInterleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC1_2BPSKInterleavingWithoutScrambling(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantCodes::ofdmBPSKInterleaving, nullptr);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4BPSKInterleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmBPSKInterleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC1_2QPSKInterleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantCodes::ofdmQPSKInterleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4QPSKInterleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmQPSKInterleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC1_2QAM16Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OfdmCompliantCodes::ofdmQAM16Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM16Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmQAM16Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC2_3QAM64Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode2_3, &Ieee80211OfdmCompliantCodes::ofdmQAM64Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM64Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmQAM64Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC5_6QAM64Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, &Ieee80211OfdmCompliantCodes::ofdmQAM64Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM256Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmQAM256Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC5_6QAM256Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, &Ieee80211OfdmCompliantCodes::ofdmQAM256Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC3_4QAM1024Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode3_4, &Ieee80211OfdmCompliantCodes::ofdmQAM1024Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);
const Ieee80211OfdmCode Ieee80211OfdmCompliantCodes::ofdmCC5_6QAM1024Interleaving(&Ieee80211OfdmCompliantCodes::ofdmConvolutionalCode5_6, &Ieee80211OfdmCompliantCodes::ofdmQAM1024Interleaving, &Ieee80211OfdmCompliantCodes::ofdmScrambling);

} /* namespace physicallayer */
} /* namespace inet */

