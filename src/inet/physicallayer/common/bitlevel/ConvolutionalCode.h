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

#ifndef __INET_CONVOLUTIONALCODE_H
#define __INET_CONVOLUTIONALCODE_H

#include "inet/physicallayer/contract/bitlevel/IFecCoder.h"

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
    int memory;
    const char *mode;

  public:
    ConvolutionalCode(const char *transferFunctionMatrix, const char *puncturingMatrix, const char *constraintLengthVector, int codeRatePuncturingK, int codeRatePuncturingN, const char *mode);

    int getCodeRatePuncturingK() const { return codeRatePuncturingK; }
    int getCodeRatePuncturingN() const { return codeRatePuncturingN; }
    const char *getConstraintLengthVector() const { return constraintLengthVector; }
    const char *getMode() const { return mode; }
    const char *getPuncturingMatrix() const { return puncturingMatrix; }
    const char *getTransferFunctionMatrix() const { return transferFunctionMatrix; }
    std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual double getCodeRate() const override;
    virtual int getEncodedLength(int decodedLength) const override;
    virtual int getDecodedLength(int encodedLength) const override;
    virtual double computeNetBitErrorRate(double grossBitErrorRate) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_CONVOLUTIONALCODE_H

