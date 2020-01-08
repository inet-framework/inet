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

// TODO redesign for object NED parameters

class INET_API ConvolutionalCode : public IForwardErrorCorrection
{
  protected:
    const std::vector<std::vector<intval_t>> transferFunctionMatrix;
    const std::vector<std::vector<intval_t>> puncturingMatrix;
    const std::vector<intval_t> constraintLengthVector;
    int codeRatePuncturingK;
    int codeRatePuncturingN;
    intval_t memory;
    const char *mode;

  public:
    ConvolutionalCode(
            const std::vector<std::vector<intval_t>>& transferFunctionMatrix,
            const std::vector<std::vector<intval_t>>& puncturingMatrix,
            const std::vector<intval_t>& constraintLengthVector,
            int codeRatePuncturingK, int codeRatePuncturingN, const char *mode);

    int getCodeRatePuncturingK() const { return codeRatePuncturingK; }
    int getCodeRatePuncturingN() const { return codeRatePuncturingN; }
    const std::vector<intval_t>& getConstraintLengthVector() const { return constraintLengthVector; }
    const char *getMode() const { return mode; }
    const std::vector<std::vector<intval_t>>& getPuncturingMatrix() const { return puncturingMatrix; }
    const std::vector<std::vector<intval_t>>& getTransferFunctionMatrix() const { return transferFunctionMatrix; }
    std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual double getCodeRate() const override;
    virtual int getEncodedLength(int decodedLength) const override;
    virtual int getDecodedLength(int encodedLength) const override;
    virtual double computeNetBitErrorRate(double grossBitErrorRate) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

