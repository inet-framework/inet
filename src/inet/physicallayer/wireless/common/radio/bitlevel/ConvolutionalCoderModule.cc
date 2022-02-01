//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoderModule.h"

namespace inet {
namespace physicallayer {

Define_Module(ConvolutionalCoderModule);

void ConvolutionalCoderModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *transferFunctionMatrix = par("transferFunctionMatrix");
        const char *puncturingMatrix = par("puncturingMatrix");
        const char *constraintLengthVector = par("constraintLengthVector");
        const char *mode = par("mode");
        int codeRatePuncturingK = par("punctureK");
        int codeRatePuncturingN = par("punctureN");
        ConvolutionalCode *convolutionalCode = new ConvolutionalCode(transferFunctionMatrix, puncturingMatrix, constraintLengthVector, codeRatePuncturingK, codeRatePuncturingN, mode);
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

