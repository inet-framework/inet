//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONVOLUTIONALCODE_H
#define __INET_CONVOLUTIONALCODE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"

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
    std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double getCodeRate() const override;
    virtual int getEncodedLength(int decodedLength) const override;
    virtual int getDecodedLength(int encodedLength) const override;
    virtual double computeNetBitErrorRate(double grossBitErrorRate) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

