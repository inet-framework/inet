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

#include "inet/physicallayer/common/bitlevel/ConvolutionalCode.h"

namespace inet {
namespace physicallayer {

ConvolutionalCode::ConvolutionalCode(const char *transferFunctionMatrix, const char *puncturingMatrix, const char *constraintLengthVector, int codeRatePuncturingK, int codeRatePuncturingN, const char *mode) :
    transferFunctionMatrix(transferFunctionMatrix),
    puncturingMatrix(puncturingMatrix),
    constraintLengthVector(constraintLengthVector),
    codeRatePuncturingK(codeRatePuncturingK),
    codeRatePuncturingN(codeRatePuncturingN),
    memory(0),
    mode(mode)
{
    cStringTokenizer tokenizer(constraintLengthVector);
    while (tokenizer.hasMoreTokens())
        memory = std::max(memory, atoi(tokenizer.nextToken()) - 1);
}

std::ostream& ConvolutionalCode::printToStream(std::ostream& stream, int level) const
{
    stream << "ConvolutionalCode";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", codeRatePuncturingK = " << codeRatePuncturingK
               << ", codeRatePuncturingN = " << codeRatePuncturingN;
    return stream;
}

double ConvolutionalCode::getCodeRate() const
{
    return (double)codeRatePuncturingK / codeRatePuncturingN;
}

int ConvolutionalCode::getEncodedLength(int decodedLength) const
{
    ASSERT(decodedLength % codeRatePuncturingK == 0);
    return decodedLength * codeRatePuncturingN / codeRatePuncturingK;
}

int ConvolutionalCode::getDecodedLength(int encodedLength) const
{
    ASSERT(encodedLength % codeRatePuncturingN == 0);
    return encodedLength * codeRatePuncturingK / codeRatePuncturingN;
}

// according to http://ita.ucsd.edu/workshop/11/files/paper/paper_1887.pdf
double ConvolutionalCode::computeNetBitErrorRate(double grossBitErrorRate) const
{
    double p = grossBitErrorRate;
    double netBitErrorRate;
    if (codeRatePuncturingK == 1 && codeRatePuncturingN == 2) {
        if (memory == 1)
            netBitErrorRate = pow(p, 2) * (14 - 23 * p + 16 * pow(p, 2) + 2 * pow(p, 3) - 16 * pow(p, 4) + 8 * pow(p, 5))
                / ((1 + 3 * pow(p, 2) - 2 * pow(p, 3)) * (2 - p + 4 * pow(p, 2) - 4 * pow(p, 3)));
        else if (memory == 2)
            netBitErrorRate = 44 * pow(p, 3) + 3519.0 / 8.0 * pow(p, 4) - 14351.0 / 32.0 * pow(p, 5) - 1267079.0 / 64.0 * pow(p, 6) - 31646405.0 / 512.0 * pow(p, 7) + 978265739.0 / 2048.0 * pow(p, 8) + 3931764263.0 / 1024.0 * pow(p, 9) - 48978857681.0 / 32768.0 * pow(p, 10);
        else
            throw cRuntimeError("Not yet implemented");
    }
    else
        throw cRuntimeError("Not yet implemented");
    return std::max(std::min(netBitErrorRate, 1.0), 0.0);
}

} // namespace physicallayer
} // namespace inet

