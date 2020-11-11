//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    //delete convolutionalCoder->getForwardErrorCorrection();
    delete convolutionalCoder;
}

} /* namespace physicallayer */
} /* namespace inet */

