//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoderModule.h"

#include "inet/common/INETUtils.h"

namespace inet {
namespace physicallayer {

Define_Module(ConvolutionalCoderModule);

void ConvolutionalCoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const cValueArray *transferFunctionMatrix = check_and_cast<cValueArray*>(par("transferFunctionMatrix").objectValue());
        const cValueArray *puncturingMatrix = check_and_cast<cValueArray*>(par("puncturingMatrix").objectValue());
        auto constraintLengthVector = check_and_cast<cValueArray*>(par("constraintLengthVector").objectValue());
        const char *mode = par("mode");
        int codeRatePuncturingK = par("punctureK");
        int codeRatePuncturingN = par("punctureN");
        ConvolutionalCode *convolutionalCode = new ConvolutionalCode(
                utils::asIntMatrix(transferFunctionMatrix),
                utils::asIntMatrix(puncturingMatrix),
                constraintLengthVector->asIntVector(), codeRatePuncturingK, codeRatePuncturingN, mode);
        convolutionalCoder = new ConvolutionalCoder(convolutionalCode);
    }
}

std::ostream& ConvolutionalCoderModule::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return convolutionalCoder->printToStream(stream, level);
}

ConvolutionalCoderModule::~ConvolutionalCoderModule()
{
    delete convolutionalCoder->getForwardErrorCorrection();
    delete convolutionalCoder;
}

} /* namespace physicallayer */
} /* namespace inet */

