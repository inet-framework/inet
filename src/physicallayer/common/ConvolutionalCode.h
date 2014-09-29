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

#ifndef __INET_CONVOLUTIONALCODE_H_
#define __INET_CONVOLUTIONALCODE_H_

#include "IFECEncoder.h"

namespace inet {
namespace physicallayer {

class INET_API ConvolutionalCode : public IForwardErrorCorrection
{
    protected:
        const char *transferFunctionMatrix;
        const char *puncturingMatrix;
        const char *constraintLengthVector;
        int codeRatePuncturingK;
        int codeRatePuncturingN;
        const char *mode;

    public:
        ConvolutionalCode(const char *transferFunctionMatrix, const char *puncturingMatrix, const char *constraintLengthVector, int codeRatePuncturingK, int codeRatePuncturingN, const char *mode) :
            transferFunctionMatrix(transferFunctionMatrix), puncturingMatrix(puncturingMatrix),
            constraintLengthVector(constraintLengthVector), codeRatePuncturingK(codeRatePuncturingK),
            codeRatePuncturingN(codeRatePuncturingN), mode(mode) {}

        int getCodeRatePuncturingK() const { return codeRatePuncturingK; }
        int getCodeRatePuncturingN() const { return codeRatePuncturingN; }
        const char* getConstraintLengthVector() const { return constraintLengthVector; }
        const char* getMode() const { return mode; }
        const char* getPuncturingMatrix() const { return puncturingMatrix; }
        const char* getTransferFunctionMatrix() const { return transferFunctionMatrix; }
        void printToStream(std::ostream& stream) const
        {
            stream << codeRatePuncturingK << "/" << codeRatePuncturingN << " convolutional encoder/decoder";
        }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_CONVOLUTIONALCODE_H_ */
