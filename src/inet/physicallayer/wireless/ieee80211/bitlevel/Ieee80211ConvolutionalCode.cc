//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

Ieee80211ConvolutionalCode::Ieee80211ConvolutionalCode(int codeRateK, int codeRateN) :
    ConvolutionalCode("133 171", nullptr, "7", -1, -1, "truncated")
{
    if (codeRateK == 1 && codeRateN == 2) {
        codeRatePuncturingK = 1;
        codeRatePuncturingN = 2;
        puncturingMatrix = "1; 1";
    }
    else if (codeRateK == 2 && codeRateN == 3) {
        codeRatePuncturingK = 2;
        codeRatePuncturingN = 3;
        puncturingMatrix = "1 1; 1 0";
    }
    else if (codeRateK == 3 && codeRateN == 4) {
        codeRatePuncturingK = 3;
        codeRatePuncturingN = 4;
        puncturingMatrix = "1 1 0; 1 0 1";
    }
    else if (codeRateK == 5 && codeRateN == 6) {
        codeRatePuncturingK = 5;
        codeRatePuncturingN = 6;
        puncturingMatrix = "1 1 0 1 0; 1 0 1 0 1";
    }
    else
        throw cRuntimeError("Unsupported code rate %d/%d", codeRateK, codeRateN);
}

} // namespace physicallayer
} // namespace inet

